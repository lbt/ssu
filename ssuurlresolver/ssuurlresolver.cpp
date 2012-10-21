/**
 * @file ssuurlresolver.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QCoreApplication>
#include "ssuurlresolver.h"

SsuUrlResolver::SsuUrlResolver(): QObject(){
  logfile.setFileName("/var/log/ssu.log");
  logfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  logstream.setDevice(&logfile);
  QObject::connect(this,SIGNAL(done()),
                   QCoreApplication::instance(),SLOT(quit()),
                   Qt::QueuedConnection);
}

void SsuUrlResolver::run(){
  QHash<QString, QString> repoParameters;
  QString resolvedUrl, repo;
  bool isRnd = false;

  PluginFrame in(std::cin);

  if (in.headerEmpty()){
    // FIXME, do something; we need at least repo header
    logstream << "D'oh, received empty header list\n";
  }

  PluginFrame::HeaderListIterator it;
  QStringList headerList;

  /*
     hasKey() for some reason makes zypper run into timeouts, so we have
     to handle special keys here
  */
  for (it=in.headerBegin();it!=in.headerEnd();it++){
    QString first = QString::fromStdString((*it).first);
    QString second = QString::fromStdString((*it).second);

    if (first == "repo"){
      repo = second;
    } else if (first == "rnd"){
      isRnd = true;
    } else if (first == "deviceFamily"){
      repoParameters.insert(first, second);
    } else if (first == "arch"){
      repoParameters.insert(first, second);
    } else if (first == "debug"){
      repoParameters.insert("debugSplit", "debug");
    } else {
      if ((*it).second.empty())
        headerList.append(first);
      else
        headerList.append(QString("%1=%2")
                          .arg(first)
                          .arg(second));
    }
  }

  if (!ssu.useSslVerify())
    headerList.append("ssl_verify=no");

  if (isRnd){
    SignalWait w;
    connect(&ssu, SIGNAL(done()), &w, SLOT(finished()));
    ssu.updateCredentials();
    w.sleep();
  }

  // TODO: check for credentials scope required for repository; check if the file exists;
  //       compare with configuration, and dump credentials to file if necessary
  logstream << QString("Requesting credentials for '%1' with RND status %2...").arg(repo).arg(isRnd);
  QString credentialsScope = ssu.credentialsScope(repo, isRnd);
  if (!credentialsScope.isEmpty()){
    headerList.append(QString("credentials=%1").arg(credentialsScope));

    QFileInfo credentialsFileInfo("/etc/zypp/credentials.d/" + credentialsScope);
    if (!credentialsFileInfo.exists() ||
        credentialsFileInfo.lastModified() <= ssu.lastCredentialsUpdate()){
      QFile credentialsFile(credentialsFileInfo.filePath());
      credentialsFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
      QTextStream out(&credentialsFile);
      QPair<QString, QString> credentials = ssu.credentials(credentialsScope);
      out << "[ssu-credentials]\n";
      out << "username=" << credentials.first << "\n";
      out << "password=" << credentials.second << "\n";
      out.flush();
      credentialsFile.close();
    }
  }

  if (headerList.isEmpty()){
    resolvedUrl = ssu.repoUrl(repo, isRnd, repoParameters);
  } else {
    resolvedUrl = QString("%1?%2")
      .arg(ssu.repoUrl(repo, isRnd, repoParameters))
      .arg(headerList.join("&"));
  }

  logstream << QString("resolved to %1\n").arg(resolvedUrl);
  PluginFrame out("RESOLVEDURL");
  out.setBody(resolvedUrl.toStdString());
  out.writeTo(std::cout);

  emit done();
}