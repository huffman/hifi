#line 2

const vec3 color = vec3(0.2, 0.4, 0.8);

const float SPEED = 0.05;

vec4 getProceduralColor() {
    // Discard anything that isn't on the top of the surface
    if (_position.y < 0.5) discard;

    float d = pow(_position.x, 2) + pow(_position.z, 2);
    if (d > 0.2) discard;


    float t = mod(iGlobalTime * SPEED, 1.0);
    d -= t;

    if (d <= 0.8 && mod(d, 0.1) >= 0.08) {
        return vec4(color, 1.0);
    } else {
        discard;
    }
}
