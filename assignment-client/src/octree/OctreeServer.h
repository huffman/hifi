//
//  OctreeServer.h
//  assignment-client/src/octree
//
//  Created by Brad Hefta-Gaub on 8/21/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_OctreeServer_h
#define hifi_OctreeServer_h

#include <memory>

#include <QStringList>
#include <QDateTime>
#include <QtCore/QCoreApplication>

#include <HTTPManager.h>

#include <ThreadedAssignment.h>

#include "OctreePersistThread.h"
#include "OctreeSendThread.h"
#include "OctreeServerConsts.h"
#include "OctreeServerStats.h"
#include "OctreeInboundPacketProcessor.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(octree_server)

const int DEFAULT_PACKETS_PER_INTERVAL = 2000; // some 120,000 packets per second total

enum class OctreeServerState {
    WaitingForDomainSettings,
    WaitingForOctreeDataNegotation,
    Running
};

/// Handles assignments of type OctreeServer - sending octrees to various clients.
class OctreeServer : public ThreadedAssignment, public HTTPRequestHandler {
    Q_OBJECT
public:
    OctreeServer(ReceivedMessage& message);
    ~OctreeServer();

    static int getCurrentClientCount() { return _clientCount; }
    static void clientConnected() { _clientCount++; }
    static void clientDisconnected() { _clientCount--; }

    OctreeServerState _state { OctreeServerState::WaitingForDomainSettings };

    /// allows setting of run arguments
    void setArguments(int argc, char** argv);

    bool wantsDebugSending() const { return _debugSending; }
    bool wantsDebugReceiving() const { return _debugReceiving; }
    bool wantsVerboseDebug() const { return _verboseDebug; }

    OctreePointer getOctree() { return _tree; }

    int getPacketsPerClientPerInterval() const { return std::min(_packetsPerClientPerInterval,
                                std::max(1, getPacketsTotalPerInterval() / std::max(1, getCurrentClientCount()))); }

    int getPacketsPerClientPerSecond() const { return getPacketsPerClientPerInterval() * INTERVALS_PER_SECOND; }
    int getPacketsTotalPerInterval() const { return _packetsTotalPerInterval; }
    int getPacketsTotalPerSecond() const { return getPacketsTotalPerInterval() * INTERVALS_PER_SECOND; }

    bool isInitialLoadComplete() const { return (_persistThread) ? _persistThread->isInitialLoadComplete() : true; }
    bool isPersistEnabled() const { return (_persistThread) ? true : false; }
    quint64 getLoadElapsedTime() const { return (_persistThread) ? _persistThread->getLoadElapsedTime() : 0; }
    QString getPersistFilename() const { return (_persistThread) ? _persistThread->getPersistFilename() : ""; }
    QString getPersistFileMimeType() const { return (_persistThread) ? _persistThread->getPersistFileMimeType() : "text/plain"; }
    QByteArray getPersistFileContents() const { return (_persistThread) ? _persistThread->getPersistFileContents() : QByteArray(); }

    // Subclasses must implement these methods
    virtual std::unique_ptr<OctreeQueryNode> createOctreeQueryNode() = 0;
    virtual char getMyNodeType() const = 0;
    virtual PacketType getMyQueryMessageType() const = 0;
    virtual const char* getMyServerName() const = 0;
    virtual const char* getMyLoggingServerTargetName() const = 0;
    virtual const char* getMyDefaultPersistFilename() const = 0;
    virtual PacketType getMyEditNackType() const = 0;
    virtual QString getMyDomainSettingsKey() const { return QString("octree_server_settings"); }

    // subclass may implement these method
    virtual void beforeRun() { }
    virtual bool hasSpecialPacketsToSend(const SharedNodePointer& node) { return false; }
    virtual int sendSpecialPackets(const SharedNodePointer& node, OctreeQueryNode* queryNode, int& packetsSent) { return 0; }
    virtual QString serverSubclassStats() { return QString(); }
    virtual void trackSend(const QUuid& dataID, quint64 dataLastEdited, const QUuid& viewerNode) { }
    virtual void trackViewerGone(const QUuid& viewerNode) { }

    bool handleHTTPRequest(HTTPConnection* connection, const QUrl& url, bool skipSubHandler) override;

    virtual void aboutToFinish() override;

public slots:
    /// runs the octree server assignment
    void run() override;
    virtual void nodeAdded(SharedNodePointer node);
    virtual void nodeKilled(SharedNodePointer node);
    void sendStatsPacket() override;

private slots:
    void domainSettingsRequestComplete();
    void handleOctreeQueryPacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer senderNode);
    void handleOctreeDataNackPacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer senderNode);
    void handleOctreeDataFileReply(QSharedPointer<ReceivedMessage> message);
    void removeSendThread();

protected:
    using UniqueSendThread = std::unique_ptr<OctreeSendThread>;
    using SendThreads = std::unordered_map<QUuid, UniqueSendThread>;
    
    virtual OctreePointer createTree() = 0;
    bool readOptionBool(const QString& optionName, const QJsonObject& settingsSectionObject, bool& result);
    bool readOptionInt(const QString& optionName, const QJsonObject& settingsSectionObject, int& result);
    bool readOptionInt64(const QString& optionName, const QJsonObject& settingsSectionObject, qint64& result);
    bool readOptionString(const QString& optionName, const QJsonObject& settingsSectionObject, QString& result);
    void readConfiguration();
    virtual void readAdditionalConfiguration(const QJsonObject& settingsSectionObject) { };
    void parsePayload();
    void initHTTPManager(int port);
    QString getUptime();
    QString getFileLoadTime();
    QString getConfiguration();
    QString getStatusLink();

    void beginRunning(QByteArray replaceData);
    
    UniqueSendThread createSendThread(const SharedNodePointer& node);
    virtual UniqueSendThread newSendThread(const SharedNodePointer& node);

    int _argc;
    const char** _argv;
    char** _parsedArgV;
    QJsonObject _settings;

    bool _isShuttingDown = false;

    HTTPManager* _httpManager;
    int _statusPort;
    QString _statusHost;

    QString _persistFilePath;
    QString _persistAbsoluteFilePath;
    QString _persistAsFileType;
    QString _backupDirectoryPath;
    int _packetsPerClientPerInterval;
    int _packetsTotalPerInterval;
    OctreePointer _tree; // this IS a reaveraging tree
    bool _wantPersist;
    bool _debugSending;
    bool _debugReceiving;
    bool _debugTimestampNow;
    bool _verboseDebug;
    OctreeInboundPacketProcessor* _octreeInboundPacketProcessor;
    OctreePersistThread* _persistThread;

    int _persistInterval;
    bool _wantBackup;
    bool _persistFileDownload;
    QString _backupExtensionFormat;
    int _backupInterval;
    int _maxBackupVersions;

    time_t _started;
    quint64 _startedUSecs;
    QString _safeServerName;
    
    SendThreads _sendThreads;

    static int _clientCount;
};

#endif // hifi_OctreeServer_h
