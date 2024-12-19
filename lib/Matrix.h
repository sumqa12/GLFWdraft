#pragma once
#include <cmath>
#include <algorithm>
#include <GL/glew.h>

// 変換行列
class Matrix
{
    // 変換行列の要素
    GLfloat matrix[16];

public:
    // コンストラクタ
    Matrix() {}

    // 配列の内容で初期化するコンストラクタ
    //  a: GLfloat型の16要素の配列
    Matrix(const GLfloat *a)
    {
        std::copy(a, a + 16, matrix);
    }

    // 行列の要素を右辺値として参照する 
    const GLfloat &operator[](std::size_t i) const 
    { 
        return matrix[i];       
    } 
    
    // 行列の要素を左辺値として参照する 
    GLfloat &operator[](std::size_t i) 
    { 
        return matrix[i]; 
    }

    // 変換行列の配列を返す
    const GLfloat *data() const
    {
        return matrix;
    }

    // 法線ベクトルの変換行列を求める 
    void getNormalMatrix(GLfloat *m) const 
    { 
        m[0] = matrix[ 5] * matrix[10] - matrix[ 6] * matrix[ 9]; 
        m[1] = matrix[ 6] * matrix[ 8] - matrix[ 4] * matrix[10]; 
        m[2] = matrix[ 4] * matrix[ 9] - matrix[ 5] * matrix[ 8]; 
        m[3] = matrix[ 9] * matrix[ 2] - matrix[10] * matrix[ 1]; 
        m[4] = matrix[10] * matrix[ 0] - matrix[ 8] * matrix[ 2]; 
        m[5] = matrix[ 8] * matrix[ 1] - matrix[ 9] * matrix[ 0]; 
        m[6] = matrix[ 1] * matrix[ 6] - matrix[ 2] * matrix[ 5]; 
        m[7] = matrix[ 2] * matrix[ 4] - matrix[ 0] * matrix[ 6]; 
        m[8] = matrix[ 0] * matrix[ 5] - matrix[ 1] * matrix[ 4]; 
    }

    // 単位行列を設定
    void loadIdentity()
    {
        std::fill(matrix, matrix + 16, 0.0f);
        matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
    }

    // 単位行列を作成
    static Matrix identity()
    {
        Matrix t;
        t.loadIdentity();
        return t;
    }

    // (x, y, z)だけ平行移動する変換行列を作成
    static Matrix translate(GLfloat x, GLfloat y, GLfloat z)
    {
        Matrix t;
        t.loadIdentity();
        t[12] = x;
        t[13] = y;
        t[14] = z;

        return t;
    }

    // (x, y, z)倍に拡大縮小する変換行列を作成
    static Matrix scale(GLfloat sx, GLfloat sy, GLfloat sz)
    {
        Matrix t;

        t.loadIdentity();
        t[ 0] = sx;
        t[ 5] = sy;
        t[10] = sz;

        return t;
    }

    // 任意の点を中心をした拡大縮小
    static Matrix scalePoint(
        GLfloat x, GLfloat y, GLfloat z,
        GLfloat sx, GLfloat sy, GLfloat sz)
    {
        Matrix t, s, mt;
        t.loadIdentity();
        s.loadIdentity();
        mt.loadIdentity();

        t = translate(x, y, z);
        s = scale(sx, sy, sz);
        mt = translate(-x, -y, -z);

        return t * s * mt;
    }

    // せん断する変換行列を作成
    //  magnification 係数
    //  mode {
    //      1:  Hxy(magnification)
    //      2:  Hyx(magnification)
    //      3:  Hyz(magnification)
    //      4:  Hzy(magnification)
    //      5:  Hzx(magnification)
    //      6:  Hxz(magnification)
    //  }
    static Matrix shear(GLint mode, GLfloat magnification)
    {
        Matrix t;
        t.loadIdentity();

        switch (mode)
        {
        case 1:
            t[4] = magnification;
            break;
        case 2:
            t[1] = magnification;
            break;
        case 3:
            t[9] = magnification;
            break;
        case 4:
            t[6] = magnification;
            break;
        case 5:
            t[2] = magnification;
            break;
        case 6:
            t[8] = magnification;
            break;
        default:
            break;
        }
        return t;
    }
    // 座標軸(mode)を軸にa回転する変換行列を作成
    //  mode {
    //      1: x軸
    //      2: y軸
    //      3: z軸
    //  }
    static Matrix rotate(int mode, GLfloat a)
    {
        Matrix t;
        t.loadIdentity();
        if (!(mode == 1 || mode == 2 || mode == 3)) return t;

        GLfloat fcos(cos(a));
        GLfloat fsin(sin(a));

        switch (mode)
        {
        case 1:
            t[ 5] = fcos;
            t[ 6] = fsin;
            t[ 9] = -fsin;
            t[10] = fcos;

            break;
        case 2:
            t[ 0] = fcos;
            t[ 2] = -fsin;
            t[ 8] = fsin;
            t[10] = fcos;

            break;
        case 3:
            t[0] = fcos;
            t[1] = fsin;
            t[4] = -fsin;
            t[5] = fcos;

            break;
        default:
            break;
        }

        return t;
    }

    // (x, z, y)を軸にa回転する変換行列を作成
    static Matrix rotateAxis(GLfloat a, GLfloat x, GLfloat y, GLfloat z)
    {
        Matrix t;
        const GLfloat d(sqrt(x * x + y * y + z * z));

        if (d > 0.0f)
        {
            const GLfloat l(x / d), m(y / d), n(z / d);
            const GLfloat l2(l * l), m2(m * m), n2(n * n);
            const GLfloat lm(l * m), mn(m * n), nl(n * l);
            const GLfloat c(cos(a)), c1(1.0f - c), s(sin(a));

            t.loadIdentity();
            t[ 0] = (1.0f - l2) * c + l2;
            t[ 1] = lm * c1 + n * s;
            t[ 2] = nl * c1 - m * s;
            t[ 4] = lm * c1 - n * s;
            t[ 5] = (1.0f - m2) * c + m2;
            t[ 6] = mn * c1 + l * s;
            t[ 8] = nl * c1 + m * s;
            t[ 9] = mn * c1 - l * s;
            t[10] = (1.0f - n2) * c + n2;
        }

        return t;
    }

    // ローカル座標系を回転する変換行列を作成
    static Matrix localRotate(GLfloat p, GLfloat h, GLfloat r) {
        Matrix rx, ry, rz;
        rx = rotate(1, p);
        ry = rotate(2, h);
        rz = rotate(3, r);

        const int size = 16;
        const GLfloat* data = rx.data();

        return rx * ry * rz;
    }

    // 任意の点を中心をした回転をする変換行列を作成
    //  mode {
    //  1: x軸
    //  2: y軸
    //  3: z軸
    //  }
    static Matrix rotatePoint(GLfloat a, GLfloat x, GLfloat y, GLfloat z)
    {
        Matrix t, r, mt;
        t.loadIdentity();

        t = translate(x, y, z);
        r = rotate(3, a);
        mt = translate(-x, -y, -z);

        return t * r * mt;
    }

    // ビュー変換行列を作成
    static Matrix lookat(
        GLfloat ex, GLfloat ey, GLfloat ez,   // 視点の位置 
        GLfloat gx, GLfloat gy, GLfloat gz,   // 目標点の位置 
        GLfloat ux, GLfloat uy, GLfloat uz)   // 上方向のベクトル 
    {
        // 平行移の変換行列
        const Matrix tv(translate(-ex, -ey, -ez));

        // t 軸 = e - g 
        const GLfloat tx(ex - gx); 
        const GLfloat ty(ey - gy); 
        const GLfloat tz(ez - gz); 
    
        // r 軸 = u x t 軸 
        const GLfloat rx(uy * tz - uz * ty); 
        const GLfloat ry(uz * tx - ux * tz); 
        const GLfloat rz(ux * ty - uy * tx); 
    
        // s 軸 = t 軸 x r 軸 
        const GLfloat sx(ty * rz - tz * ry); 
        const GLfloat sy(tz * rx - tx * rz); 
        const GLfloat sz(tx * ry - ty * rx);

        // s 軸の長さのチェック 
        const GLfloat s2(sx * sx + sy * sy + sz * sz); 
        if (s2 == 0.0f) return tv; 
    
        // 回転の変換行列 
        Matrix rv; 
        rv.loadIdentity(); 
    
        // r 軸を正規化して配列変数に格納 
        const GLfloat r(sqrt(rx * rx + ry * ry + rz * rz)); 
        rv[ 0] = rx / r;
        rv[ 4] = ry / r; 
        rv[ 8] = rz / r; 

        // s 軸を正規化して配列変数に格納 
        const GLfloat s(sqrt(s2)); 
        rv[ 1] = sx / s; 
        rv[ 5] = sy / s; 
        rv[ 9] = sz / s; 

        // t 軸を正規化して配列変数に格納 
        const GLfloat t(sqrt(tx * tx + ty * ty + tz * tz)); 
        rv[ 2] = tx / t; 
        rv[ 6] = ty / t; 
        rv[10] = tz / t;

        // 視点の平行移動の変換行列に視線の回転の変換行列をかける
        return rv * tv;
    }

    // 直行投影変換行列を作成
    static Matrix orthogonal(
        GLfloat left, GLfloat right,
        GLfloat bottom, GLfloat top,
        GLfloat zNear, GLfloat zFar)
    {
        Matrix t;
        const GLfloat dx(right - left);
        const GLfloat dy(top - bottom);
        const GLfloat dz(zFar - zNear);

        if (dx != 0.0f && dy != 0.0f && dz != 0.0f) {
            t.loadIdentity();
            t[ 0] = 2.0f/ dx;
            t[ 5] = 2.0f / dy;
            t[10] = -2.0f / dz;
            t[12] = -(right + left) / dx;
            t[13] = -(top + bottom) / dy;
            t[14] = -(zFar + zNear) / dz;
        }

        return t;
    }

    // 透視投影変換行列を作成する 
    static Matrix frustum(
        GLfloat left, GLfloat right, 
        GLfloat bottom, GLfloat top, 
        GLfloat zNear, GLfloat zFar)
    {
        Matrix t;
        const GLfloat dx(right - left);
        const GLfloat dy(top - bottom);
        const GLfloat dz(zFar - zNear);

        if (dx != 0.0f && dy != 0.0f && dz != 0.0f) {
            t.loadIdentity();
            t[ 0] = 2.0f * zNear / dx;
            t[ 5] = 2.0f * zNear / dy;
            t[ 8] = (right + left) / dx;
            t[ 9] = (top + bottom) / dy;
            t[10] = -(zFar + zNear) / dz;
            t[11] = -1.0f;
            t[14] = -2.0f * zFar * zNear / dz;
            t[15] = 0.0f;
        }

        return t;
    }

    // 画角を指定して透視投影変換行列を作成
    static Matrix perspective(
        GLfloat fovy, GLfloat aspect,
        GLfloat zNear, GLfloat zFar)
    {
        Matrix t;
        const GLfloat dz(zFar - zNear);

        if (dz != 0.0f) {
            t.loadIdentity();
            t[ 5] = 1.0f / tan(fovy * 0.5f);
            t[ 0] = t[5] / aspect;
            t[10] = -(zFar + zNear) / dz;
            t[11] = -1.0f;
            t[14] = -2.0f * zFar * zNear / dz;
            t[15] = 0.0f;
        }

        return t;
    }

    // 転置行列を求める
    static Matrix transpose(Matrix &m) {
        Matrix t;
        t.loadIdentity();

        for (int i = 0, j = 0; i <= 8; i += 4, j++) {
            t[  i  ] = m[j];
            t[i + 1] = m[j + 4];
            t[i + 2] = m[j + 8];
        }

        return t;
    }

    // 乗算
    Matrix operator*(const Matrix &m) const
    {
        Matrix t;

        for (int i = 0; i < 16; ++i)
        {
            const int j(i & 3), k(i & ~3);
            t[i] =
                matrix[ 0 + j] * m[k + 0] +
                matrix[ 4 + j] * m[k + 1] +
                matrix[ 8 + j] * m[k + 2] +
                matrix[12 + j] * m[k + 3];
        }

        return t;
    }
};