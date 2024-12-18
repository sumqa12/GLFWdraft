#pragma once
#include <memory>
#include <GL/glew.h>

// ユニフォームバッファオブジェクト
template<typename Type>
class Uniform {
    struct UniformBuffer{
        // ユニフォームバッファオブジェクト名
        GLuint ubo;

        // コンストラクタ
        //  data: uniformプロックに格納するデータ
        UniformBuffer(const Type *data) {
            // ユニフォームバッファオブジェクトを作成
            glGenBuffers(1, &ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo);
            glBufferData(
                GL_UNIFORM_BUFFER , sizeof (Type), data, GL_STATIC_DRAW
            );
        }

        // デストラクタ
        ~UniformBuffer() {
            // ユニフォームバッファオブジェクトを削除
            glDeleteBuffers(1, &ubo);
        }
    };

    // バッファオブジェクト
    const std::shared_ptr<const UniformBuffer> buffer;

public:

    // コンストラクタ
    //  data: uniformブロックに格納するデータ
    Uniform(const Type *data = NULL)
        : buffer(new UniformBuffer(data))
    {}

    // デストラクタ
    virtual ~Uniform()
    {}

    // ユニフォームバッファオブジェクトにデータを格納
    //  data: uniformブロックに格納するデータ
    void set(const Type *data) const {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer->ubo);
        glBufferSubData(
            GL_UNIFORM_BUFFER, 0, sizeof (Type), data
        );
    }

    // このユニフォームバッファオブジェクトを使用
    //  bp: 結合ポイント
    void select(GLuint bp) const {
        // 材質に設定するユニフォームバッファオブジェクトを指定
        glBindBufferBase(
            GL_UNIFORM_BUFFER, bp, buffer->ubo
        );
    }
};