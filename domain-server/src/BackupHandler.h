//
//  BackupHandler.h
//  assignment-client
//
//  Created by Clement Brisset on 2/5/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_BackupHandler_h
#define hifi_BackupHandler_h

#include <memory>

#include <QDebug>

#include <quazip5/quazip.h>

class BackupHandler {
public:
    template <typename T>
    BackupHandler(T x) : _self(std::make_shared<Model<T>>(std::move(x))) {}

    void loadBackup(QuaZip& zip) {
        _self->loadBackup(zip);
    }
    void createBackup(QuaZip& zip) {
        _self->createBackup(zip);
    }
    void recoverBackup(QuaZip& zip) {
        _self->recoverBackup(zip);
    }
    void deleteBackup(QuaZip& zip) {
        _self->deleteBackup(zip);
    }
    void consolidateBackup(QuaZip& zip) {
        _self->consolidateBackup(zip);
    }

private:
    struct Concept {
        virtual ~Concept() = default;

        virtual void loadBackup(QuaZip& zip) = 0;
        virtual void createBackup(QuaZip& zip) = 0;
        virtual void recoverBackup(QuaZip& zip) = 0;
        virtual void deleteBackup(QuaZip& zip) = 0;
        virtual void consolidateBackup(QuaZip& zip) = 0;
    };

    template <typename T>
    struct Model : Concept {
        Model(T x) : data(std::move(x)) {}

        void loadBackup(QuaZip& zip) {
            data.loadBackup(zip);
        }
        void createBackup(QuaZip& zip) {
            data.createBackup(zip);
        }
        void recoverBackup(QuaZip& zip) {
            data.recoverBackup(zip);
        }
        void deleteBackup(QuaZip& zip) {
            data.deleteBackup(zip);
        }
        void consolidateBackup(QuaZip& zip) {
            data.consolidateBackup(zip);
        }

        T data;
    };

    std::shared_ptr<Concept> _self;
};

#include <quazip5/quazipfile.h>
class EntitiesBackupHandler {
public:
    EntitiesBackupHandler(QString entitiesFilePath) : _entitiesFilePath(entitiesFilePath) {}

    void loadBackup(QuaZip& zip) {
    }

    // Create a skeleton backup
    void createBackup(QuaZip& zip) {
        qDebug() << "Creating a backup from handler";

        QFile entitiesFile { _entitiesFilePath };

        if (entitiesFile.open(QIODevice::ReadOnly)) {
            QuaZipFile zipFile { &zip };
            zipFile.open(QIODevice::WriteOnly, QuaZipNewInfo("models.json.gz", _entitiesFilePath));
            zipFile.write(entitiesFile.readAll());
            zipFile.close();
            if (zipFile.getZipError() != UNZ_OK) {
                qDebug() << "testCreate(): outFile.close(): " << zipFile.getZipError();
            }
        }
    }

    // Recover from a full backup
    void recoverBackup(QuaZip& zip) {
        if (!zip.setCurrentFile("models.json.gz")) {
            qWarning() << "Failed to find models.json.gz while recovering backup";
            return;
        }
        QuaZipFile zipFile { &zip };
        zipFile.open(QIODevice::ReadOnly);
        auto data = zipFile.readAll();

        QFile entitiesFile { _entitiesFilePath };

        if (entitiesFile.open(QIODevice::WriteOnly)) {
            entitiesFile.write(data);
        }

        zipFile.close();
    }

    // Delete a skeleton backup
    void deleteBackup(QuaZip& zip) {
    }

    // Create a full backup
    void consolidateBackup(QuaZip& zip) {
    }

private:
    QString _entitiesFilePath;
};

#endif /* hifi_BackupHandler_h */
