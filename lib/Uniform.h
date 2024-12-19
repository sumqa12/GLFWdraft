#pragma once
#include <memory>
#include <GL/glew.h>

// ユニフォームバッファオブジェクト
template<typename Type>
class Uniform {
    struct UniformBuffer{
        // ユニフォームバッファオブジェクト名
        GLuint ubo;
        
        // ユニフォームブロックのサイズ
        GLsizeiptr blocksize;

        // コンストラクタ
        //  data: uniformプロックに格納するデータ
        //  count: 確保するuniformブロックの数
        UniformBuffer(const Type *data, unsigned int count) {
            // ユニフォームブロックのサイズを求める
            GLint alignment;
            glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
            blocksize = (((sizeof (Type) - 1) / alignment) + 1) * alignment;

            // ユニフォームバッファオブジェクトを作成
            glGenBuffers(1, &ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo);
            glBufferData(
                GL_UNIFORM_BUFFER , count * blocksize, NULL, GL_STATIC_DRAW
            );
            for (unsigned int i = 0; i < count; i++) {
                glBufferSubData(
                    GL_UNIFORM_BUFFER, i * blocksize, sizeof (Type), data + i
                );
            }
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
    //  count: 確保するuniformブロックの数
    Uniform(const Type *data = NULL, unsigned int count = 1)
        : buffer(new UniformBuffer(data, count))
    {}

    // デストラクタ
    virtual ~Uniform()
    {}

    // ユニフォームバッファオブジェクトにデータを格納
    //  data: uniformブロックに格納するデータ
    //  start: データを格納するuniformブロックの先頭位置
    //  count: データを格納するuniformプロックの数
    void set(const Type *data, unsigned int start = 0, unsigned int count = 1) const {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer->ubo);
        for (unsigned int i = 0; i < count; i++) {
            glBufferSubData(
                GL_UNIFORM_BUFFER, (start + i) * buffer->blocksize,
                sizeof (Type), data + i
            );
        }
    }

    // このユニフォームバッファオブジェクトを使用
    //  bp: 結合ポイント
    //  i : 結合するuniformブロックの位置 
    void select(GLuint bp, unsigned int i = 0) const {
        // 材質に設定するユニフォームバッファオブジェクトを指定
        glBindBufferRange(
            GL_UNIFORM_BUFFER, bp, buffer->ubo,
            i * buffer->blocksize, sizeof (Type)
        );
    }
};