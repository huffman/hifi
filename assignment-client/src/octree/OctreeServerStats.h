#include <SimpleMovingAverage.h>

#include <QMap>
#include <QMutex>

class OctreeSendThread;

class OctreeServerStats {
public:
    static void resetSendingStats();

    static constexpr float SKIP_TIME = -1.0f; // use this for trackXXXTime() calls for non-times

    static void trackLoopTime(float time) { _averageLoopTime.updateAverage(time); }
    static float getAverageLoopTime() { return _averageLoopTime.getAverage(); }

    static void trackEncodeTime(float time);
    static float getAverageEncodeTime() { return _averageEncodeTime.getAverage(); }

    static void trackInsideTime(float time) { _averageInsideTime.updateAverage(time); }
    static float getAverageInsideTime() { return _averageInsideTime.getAverage(); }

    static void trackTreeWaitTime(float time);
    static float getAverageTreeWaitTime() { return _averageTreeWaitTime.getAverage(); }

    static void trackTreeTraverseTime(float time) { _averageTreeTraverseTime.updateAverage(time); }
    static float getAverageTreeTraverseTime() { return _averageTreeTraverseTime.getAverage(); }

    static void trackNodeWaitTime(float time) { _averageNodeWaitTime.updateAverage(time); }
    static float getAverageNodeWaitTime() { return _averageNodeWaitTime.getAverage(); }

    static void trackCompressAndWriteTime(float time);
    static float getAverageCompressAndWriteTime() { return _averageCompressAndWriteTime.getAverage(); }

    static void trackPacketSendingTime(float time);
    static float getAveragePacketSendingTime() { return _averagePacketSendingTime.getAverage(); }

    static void trackProcessWaitTime(float time);
    static float getAverageProcessWaitTime() { return _averageProcessWaitTime.getAverage(); }

    // these methods allow us to track which threads got to various states
    static void didProcess(OctreeSendThread* thread);
    static void didPacketDistributor(OctreeSendThread* thread);
    static void didHandlePacketSend(OctreeSendThread* thread);
    static void didCallWriteDatagram(OctreeSendThread* thread);
    static void stopTrackingThread(OctreeSendThread* thread);

    static int howManyThreadsDidProcess(uint64_t since = 0);
    static int howManyThreadsDidPacketDistributor(uint64_t since = 0);
    static int howManyThreadsDidHandlePacketSend(uint64_t since = 0);
    static int howManyThreadsDidCallWriteDatagram(uint64_t since = 0);

    static SimpleMovingAverage _averageLoopTime;

    static SimpleMovingAverage _averageEncodeTime;
    static SimpleMovingAverage _averageShortEncodeTime;
    static SimpleMovingAverage _averageLongEncodeTime;
    static SimpleMovingAverage _averageExtraLongEncodeTime;
    static int _extraLongEncode;
    static int _longEncode;
    static int _shortEncode;
    static int _noEncode;

    static SimpleMovingAverage _averageInsideTime;

    static SimpleMovingAverage _averageTreeWaitTime;
    static SimpleMovingAverage _averageTreeShortWaitTime;
    static SimpleMovingAverage _averageTreeLongWaitTime;
    static SimpleMovingAverage _averageTreeExtraLongWaitTime;
    static int _extraLongTreeWait;
    static int _longTreeWait;
    static int _shortTreeWait;
    static int _noTreeWait;

    static SimpleMovingAverage _averageTreeTraverseTime;

    static SimpleMovingAverage _averageNodeWaitTime;

    static SimpleMovingAverage _averageCompressAndWriteTime;
    static SimpleMovingAverage _averageShortCompressTime;
    static SimpleMovingAverage _averageLongCompressTime;
    static SimpleMovingAverage _averageExtraLongCompressTime;
    static int _extraLongCompress;
    static int _longCompress;
    static int _shortCompress;
    static int _noCompress;

    static SimpleMovingAverage _averagePacketSendingTime;
    static int _noSend;

    static SimpleMovingAverage _averageProcessWaitTime;
    static SimpleMovingAverage _averageProcessShortWaitTime;
    static SimpleMovingAverage _averageProcessLongWaitTime;
    static SimpleMovingAverage _averageProcessExtraLongWaitTime;
    static int _extraLongProcessWait;
    static int _longProcessWait;
    static int _shortProcessWait;
    static int _noProcessWait;

    static QMap<OctreeSendThread*, uint64_t> _threadsDidProcess;
    static QMap<OctreeSendThread*, uint64_t> _threadsDidPacketDistributor;
    static QMap<OctreeSendThread*, uint64_t> _threadsDidHandlePacketSend;
    static QMap<OctreeSendThread*, uint64_t> _threadsDidCallWriteDatagram;

    static QMutex _threadsDidProcessMutex;
    static QMutex _threadsDidPacketDistributorMutex;
    static QMutex _threadsDidHandlePacketSendMutex;
    static QMutex _threadsDidCallWriteDatagramMutex;
};
