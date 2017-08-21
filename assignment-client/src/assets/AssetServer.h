//
//  AssetServer.h
//  assignment-client/src/assets
//
//  Created by Ryan Huffman on 2015/07/21
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AssetServer_h
#define hifi_AssetServer_h

#include <QtCore/QDir>
#include <QtCore/QThreadPool>
#include <QRunnable>

#include <ThreadedAssignment.h>

#include "AssetUtils.h"
#include "ReceivedMessage.h"

class BakeAssetTask : public QObject, public QRunnable {
    Q_OBJECT
public:
    BakeAssetTask(const QString& assetHash, const QString& assetPath, const QString& filePath);

    void run() override;

signals:
    void bakeComplete(QString assetHash, QString assetPath, QVector<QString> outputFiles);

private:
    QString _assetHash;
    QString _assetPath;
    QString _filePath;
};

class AssetServer : public ThreadedAssignment {
    Q_OBJECT
public:
    AssetServer(ReceivedMessage& message);

public slots:
    void run() override;

private slots:
    void completeSetup();

    void handleAssetGetInfo(QSharedPointer<ReceivedMessage> packet, SharedNodePointer senderNode);
    void handleAssetGet(QSharedPointer<ReceivedMessage> packet, SharedNodePointer senderNode);
    void handleAssetUpload(QSharedPointer<ReceivedMessage> packetList, SharedNodePointer senderNode);
    void handleAssetMappingOperation(QSharedPointer<ReceivedMessage> message, SharedNodePointer senderNode);

    void sendStatsPacket() override;

private:
    using Mappings = QVariantHash;

    void handleGetMappingOperation(ReceivedMessage& message, SharedNodePointer senderNode, NLPacketList& replyPacket);
    void handleGetAllMappingOperation(ReceivedMessage& message, SharedNodePointer senderNode, NLPacketList& replyPacket);
    void handleSetMappingOperation(ReceivedMessage& message, SharedNodePointer senderNode, NLPacketList& replyPacket);
    void handleDeleteMappingsOperation(ReceivedMessage& message, SharedNodePointer senderNode, NLPacketList& replyPacket);
    void handleRenameMappingOperation(ReceivedMessage& message, SharedNodePointer senderNode, NLPacketList& replyPacket);

    // Mapping file operations must be called from main assignment thread only
    bool loadMappingsFromFile();
    bool writeMappingsToFile();

    /// Set the mapping for path to hash
    bool setMapping(AssetPath path, AssetHash hash);

    /// Delete mapping `path`. Returns `true` if deletion of mappings succeeds, else `false`.
    bool deleteMappings(AssetPathList& paths);

    /// Rename mapping from `oldPath` to `newPath`. Returns true if successful
    bool renameMapping(AssetPath oldPath, AssetPath newPath);

    /// Delete any unmapped files from the local asset directory
    void cleanupUnmappedFiles();

    QString getPathToAssetHash(const AssetHash& assetHash);

    void bakeAssets();
    bool needsToBeBaked(const AssetPath& path, const AssetHash& assetHash);
    void bakeAsset(const QString& assetHash, const QString& assetPath, const QString& filePath);

    /// Move baked content for asset to baked directory and update baked status
    void handleCompletedBake(AssetHash originalAssetHash, QVector<QString> bakedFilePaths);

    /// Create meta file to describe baked content for original asset
    bool createMetaFile(AssetHash originalAssetHash);

    Mappings _fileMappings;

    QDir _resourcesDirectory;
    QDir _filesDirectory;

    /// Task pool for handling uploads and downloads of assets
    QThreadPool _transferTaskPool;

    QHash<QString, std::shared_ptr<BakeAssetTask>> _pendingBakes;
    QThreadPool _bakingTaskPool;
};

#endif
