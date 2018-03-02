#include "OctreeServerStats.h"

constexpr int MOVING_AVERAGE_SAMPLE_COUNTS = 1000;

SimpleMovingAverage OctreeServerStats::_averageLoopTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageInsideTime(MOVING_AVERAGE_SAMPLE_COUNTS);

SimpleMovingAverage OctreeServerStats::_averageEncodeTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageShortEncodeTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageLongEncodeTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageExtraLongEncodeTime(MOVING_AVERAGE_SAMPLE_COUNTS);
int OctreeServerStats::_extraLongEncode = 0;
int OctreeServerStats::_longEncode = 0;
int OctreeServerStats::_shortEncode = 0;
int OctreeServerStats::_noEncode = 0;

SimpleMovingAverage OctreeServerStats::_averageTreeWaitTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageTreeShortWaitTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageTreeLongWaitTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageTreeExtraLongWaitTime(MOVING_AVERAGE_SAMPLE_COUNTS);
int OctreeServerStats::_extraLongTreeWait = 0;
int OctreeServerStats::_longTreeWait = 0;
int OctreeServerStats::_shortTreeWait = 0;
int OctreeServerStats::_noTreeWait = 0;

SimpleMovingAverage OctreeServerStats::_averageTreeTraverseTime(MOVING_AVERAGE_SAMPLE_COUNTS);

SimpleMovingAverage OctreeServerStats::_averageNodeWaitTime(MOVING_AVERAGE_SAMPLE_COUNTS);

SimpleMovingAverage OctreeServerStats::_averageCompressAndWriteTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageShortCompressTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageLongCompressTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageExtraLongCompressTime(MOVING_AVERAGE_SAMPLE_COUNTS);
int OctreeServerStats::_extraLongCompress = 0;
int OctreeServerStats::_longCompress = 0;
int OctreeServerStats::_shortCompress = 0;
int OctreeServerStats::_noCompress = 0;

SimpleMovingAverage OctreeServerStats::_averagePacketSendingTime(MOVING_AVERAGE_SAMPLE_COUNTS);
int OctreeServerStats::_noSend = 0;

SimpleMovingAverage OctreeServerStats::_averageProcessWaitTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageProcessShortWaitTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageProcessLongWaitTime(MOVING_AVERAGE_SAMPLE_COUNTS);
SimpleMovingAverage OctreeServerStats::_averageProcessExtraLongWaitTime(MOVING_AVERAGE_SAMPLE_COUNTS);
int OctreeServerStats::_extraLongProcessWait = 0;
int OctreeServerStats::_longProcessWait = 0;
int OctreeServerStats::_shortProcessWait = 0;
int OctreeServerStats::_noProcessWait = 0;

void OctreeServerStats::resetSendingStats() {
    _averageLoopTime.reset();

    _averageEncodeTime.reset();
    _averageShortEncodeTime.reset();
    _averageLongEncodeTime.reset();
    _averageExtraLongEncodeTime.reset();
    _extraLongEncode = 0;
    _longEncode = 0;
    _shortEncode = 0;
    _noEncode = 0;

    _averageInsideTime.reset();
    _averageTreeWaitTime.reset();
    _averageTreeShortWaitTime.reset();
    _averageTreeLongWaitTime.reset();
    _averageTreeExtraLongWaitTime.reset();
    _extraLongTreeWait = 0;
    _longTreeWait = 0;
    _shortTreeWait = 0;
    _noTreeWait = 0;

    _averageTreeTraverseTime.reset();

    _averageNodeWaitTime.reset();

    _averageCompressAndWriteTime.reset();
    _averageShortCompressTime.reset();
    _averageLongCompressTime.reset();
    _averageExtraLongCompressTime.reset();
    _extraLongCompress = 0;
    _longCompress = 0;
    _shortCompress = 0;
    _noCompress = 0;

    _averagePacketSendingTime.reset();
    _noSend = 0;

    _averageProcessWaitTime.reset();
    _averageProcessShortWaitTime.reset();
    _averageProcessLongWaitTime.reset();
    _averageProcessExtraLongWaitTime.reset();
    _extraLongProcessWait = 0;
    _longProcessWait = 0;
    _shortProcessWait = 0;
    _noProcessWait = 0;
}

void OctreeServerStats::trackEncodeTime(float time) {
    const float MAX_SHORT_TIME = 10.0f;
    const float MAX_LONG_TIME = 100.0f;

    if (time == SKIP_TIME) {
        _noEncode++;
    } else {
        if (time <= MAX_SHORT_TIME) {
            _shortEncode++;
            _averageShortEncodeTime.updateAverage(time);
        } else if (time <= MAX_LONG_TIME) {
            _longEncode++;
            _averageLongEncodeTime.updateAverage(time);
        } else {
            _extraLongEncode++;
            _averageExtraLongEncodeTime.updateAverage(time);
        }
        _averageEncodeTime.updateAverage(time);
    }
}

void OctreeServerStats::trackTreeWaitTime(float time) {
    const float MAX_SHORT_TIME = 10.0f;
    const float MAX_LONG_TIME = 100.0f;
    if (time == SKIP_TIME) {
        _noTreeWait++;
    } else {
        if (time <= MAX_SHORT_TIME) {
            _shortTreeWait++;
            _averageTreeShortWaitTime.updateAverage(time);
        } else if (time <= MAX_LONG_TIME) {
            _longTreeWait++;
            _averageTreeLongWaitTime.updateAverage(time);
        } else {
            _extraLongTreeWait++;
            _averageTreeExtraLongWaitTime.updateAverage(time);
        }
        _averageTreeWaitTime.updateAverage(time);
    }
}

void OctreeServerStats::trackCompressAndWriteTime(float time) {
    const float MAX_SHORT_TIME = 10.0f;
    const float MAX_LONG_TIME = 100.0f;
    if (time == SKIP_TIME) {
        _noCompress++;
    } else {
        if (time <= MAX_SHORT_TIME) {
            _shortCompress++;
            _averageShortCompressTime.updateAverage(time);
        } else if (time <= MAX_LONG_TIME) {
            _longCompress++;
            _averageLongCompressTime.updateAverage(time);
        } else {
            _extraLongCompress++;
            _averageExtraLongCompressTime.updateAverage(time);
        }
        _averageCompressAndWriteTime.updateAverage(time);
    }
}

void OctreeServerStats::trackPacketSendingTime(float time) {
    if (time == SKIP_TIME) {
        _noSend++;
    } else {
        _averagePacketSendingTime.updateAverage(time);
    }
}

void OctreeServerStats::trackProcessWaitTime(float time) {
    const float MAX_SHORT_TIME = 10.0f;
    const float MAX_LONG_TIME = 100.0f;
    if (time == SKIP_TIME) {
        _noProcessWait++;
    } else {
        if (time <= MAX_SHORT_TIME) {
            _shortProcessWait++;
            _averageProcessShortWaitTime.updateAverage(time);
        } else if (time <= MAX_LONG_TIME) {
            _longProcessWait++;
            _averageProcessLongWaitTime.updateAverage(time);
        } else {
            _extraLongProcessWait++;
            _averageProcessExtraLongWaitTime.updateAverage(time);
        }
        _averageProcessWaitTime.updateAverage(time);
    }
}
