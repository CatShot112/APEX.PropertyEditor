#pragma once

struct Vec2 {
    float x, y;

    Vec2() {
        x = y = 0.0f;
    }

    Vec2(float x, float y) {
        this->x = x;
        this->y = y;
    }
};

struct Vec3 {
    float x, y, z;

    Vec3() {
        x = y = z = 0.0f;
    }

    Vec3(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

struct Vec4 {
    float x, y, z, w;

    Vec4() {
        x = y = z = w = 0.0f;
    }

    Vec4(float x, float y, float z, float w) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
};

struct Mat3x3 {
    float a00, a01, a02;
    float b00, b01, b02;
    float c00, c01, c02;

    Mat3x3() {
        a00 = a01 = a02 = 0.0f;
        b00 = b01 = b02 = 0.0f;
        c00 = c01 = c02 = 0.0f;
    }

    Mat3x3(float a00, float a01, float a02, float b00, float b01, float b02, float c00, float c01, float c02) {
        this->a00 = a00;
        this->a01 = a01;
        this->a02 = a02;
        this->b00 = b00;
        this->b01 = b01;
        this->b02 = b02;
        this->c00 = c00;
        this->c01 = c01;
        this->c02 = c02;
    }
};

struct Mat4x4 {
    float a00, a01, a02, a03;
    float b00, b01, b02, b03;
    float c00, c01, c02, c03;
    float d00, d01, d02, d03;

    Mat4x4() {
        a00 = a01 = a02 = a03 = 0.0f;
        b00 = b01 = b02 = b03 = 0.0f;
        c00 = c01 = c02 = c03 = 0.0f;
        d00 = d01 = d02 = d03 = 0.0f;
    }

    Mat4x4(float a00, float a01, float a02, float a03, float b00, float b01, float b02, float b03, float c00, float c01, float c02, float c03, float d00, float d01, float d02, float d03) {
        this->a00 = a00;
        this->a01 = a01;
        this->a02 = a02;
        this->a03 = a03;
        this->b00 = b00;
        this->b01 = b01;
        this->b02 = b02;
        this->b03 = b03;
        this->c00 = c00;
        this->c01 = c01;
        this->c02 = c02;
        this->c03 = c03;
        this->d00 = d00;
        this->d01 = d01;
        this->d02 = d02;
        this->d03 = d03;
    }
};
