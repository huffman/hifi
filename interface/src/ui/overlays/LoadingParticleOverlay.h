//
//  LoadingParticleOverlay.h
//
//  Created by Sam Gondelman on 7/27/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_LoadingParticleOverlay_h
#define hifi_LoadingParticleOverlay_h

#include "ParticleOverlay.h"

class LoadingParticleOverlay {
public:
    LoadingParticleOverlay();

    void update();

private:
    unsigned int _overlayID;
    quint64 _particlesLastUpdatedTime { 0 };

};

#endif // hifi_LoadingParticleOverlay_h
