/**
 * @file ssudeviceinfo.h
 * @copyright 2013 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUDEVICEINFO_H
#define _SSUDEVICEINFO_H

#include <QObject>
#include <QSettings>

#include <ssusettings.h>

class SsuDeviceInfo: public QObject {
    Q_OBJECT

  public:
    SsuDeviceInfo();
    /**
     * Return the list of adaptations used for the set model
     */
    QStringList adaptationRepos();
    /**
     * Try to find the device family for the system this is running on
     */
    Q_INVOKABLE QString deviceFamily();
    /**
     * Try to find the device variant for the system this is running on.
     * If the device is not a variant it will return an empty string. If
     * fallback is set to true it return the device model in this case.
     */
    Q_INVOKABLE QString deviceVariant(bool fallback=false);
    /**
     * Try to find out ond what kind of system this is running
     */
    Q_INVOKABLE QString deviceModel();
    /**
     * Calculate the device ID used in SSU requests
     * @return QSystemDeviceInfo::imei(), if available, or QSystemDeviceInfo::uniqueDeviceID()
     */
    Q_INVOKABLE QString deviceUid();
    /**
     * Return the list of repositories explicitely disabled for this device
     */
    QStringList disabledRepos();
    /**
     * Return the complete list of repositories configured for a device
     */
    QStringList repos(bool rnd=false);
    /**
     * Override device model autodetection
     */
    Q_INVOKABLE void setDeviceModel(QString model="");
    /**
     * Return the requested variable section. 'var-' is automatically
     * prepended to the section name if not specified already.
     */
    QHash<QString, QString> variableSection(QString section);

    /**
     * Get a key from an adaptation section. Deprecated, don't use.
     */
    bool getValue(const QString& key, QString& value);

  private:
    SsuSettings *boardMappings;
    QString cachedFamily, cachedModel, cachedVariant;
};
#endif
