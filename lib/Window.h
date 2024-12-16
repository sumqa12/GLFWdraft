#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// ウィンドウ関係の処理
class Window {
    // ウィンドウのハンドル
    GLFWwindow *const window;

    // ウィンドウのサイズ
    GLfloat size[2];

    // ワールド座標系に対するデバイス座標系の拡大率
    GLfloat scale;

    // 図形の正規化デバイス座標系での位置
    GLfloat modelLoc[2];

    // 正規化視点座標系での位置
    GLfloat mouseLoc[2];

    int keyStatus;

public:

    // コンストラクタ
    Window(int width = 640, int height = 640, const char *title = "Hello! GLFW")
        : window(glfwCreateWindow(width, height, title, NULL, NULL))
        , scale(100.0f), modelLoc{ 0.0f, 0.0f }, keyStatus(GLFW_RELEASE)
    {
        if (window == NULL) {
            // 失敗
            printf("Could not create GLFW window.\n");
            exit(1);
        }

        // 現在のウィンドウを処理対象に設定
        glfwMakeContextCurrent(window);

        // GFEWを初期化
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            // 失敗
            printf("Could not initialize GLFW.\n");
            exit(1);
        }

        // 垂直同期のタイミングを待つ
        glfwSwapInterval(1);

        // このインスタンスのthisポインタを記録
        glfwSetWindowUserPointer(window, this);

        // ウィンドウのサイズ変更時
        glfwSetWindowSizeCallback(window, resize);

        // マウスホイール操作時
        glfwSetScrollCallback(window, wheel);

        // キーボード操作時
        glfwSetKeyCallback(window, keyboard);

        // 開いたウィンドウの初期設定
        resize(window, width, height);
    }

    // デストラクタ
    virtual ~Window() {
        glfwDestroyWindow(window);
    }

    // 描画ループの継続判定
    explicit operator bool() {

        // イベントを取り出す
        if (keyStatus == GLFW_RELEASE) 
            glfwWaitEvents(); 
        else 
            glfwPollEvents(); 
        
        // マウスの左ボタンの状態を調べる 
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) != GLFW_RELEASE) { 
            // マウスの左ボタンが押されていたらマウスカーソルの位置を取得する 
            double x, y; 
            glfwGetCursorPos(window, &x, &y); 
        
            // マウスカーソルの正規化デバイス座標系上での位置を求める 
            mouseLoc[0] = static_cast<GLfloat>(x) * 2.0f / size[0] - 1.0f; 
            mouseLoc[1] = -(1.0f - static_cast<GLfloat>(y) * 2.0f / size[1]);
        }

        // キーボードの入力状態を調べる
        if (glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_RELEASE) {
            modelLoc[0] -= 2.0f  / size[0];
        } else if (glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_RELEASE) {
            modelLoc[0] += 2.0f  / size[0];
        } else if (glfwGetKey(window, GLFW_KEY_DOWN) != GLFW_RELEASE) {
            modelLoc[1] -= 2.0f  / size[1];
        } else if (glfwGetKey(window, GLFW_KEY_UP) != GLFW_RELEASE) {
            modelLoc[1] += 2.0f  / size[1];
        }

        // ウィンドウを閉じる必要がなければtrueを返す
        return !glfwWindowShouldClose(window) &&
            !glfwGetKey(window, GLFW_KEY_ESCAPE);
    }

    // ダブルバッファリング
    void swapBuffers() const {
        // カラーバッファを入れ替える
        glfwSwapBuffers(window);
    }

    // ウィンドウのサイズ変更時の処理
    static void resize(GLFWwindow *const window, int width, int height) {
        // フレームバッファのサイズを調べる
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        // フレームバッファ全体をビューポートに設定
        glViewport(0, 0, fbWidth, fbHeight);

        // このインスタンスのthisポインタを取得
        Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

        if (instance != NULL) {
            // 開いたウィンドウのサイズを保持
            instance->size[0] = static_cast<GLfloat>(width);
            instance->size[1] = static_cast<GLfloat>(height);
        }
    }
    
    // マウスホイール操作時の処理
    static void wheel(GLFWwindow *window, double x, double y) {
        // このインスタンスのthisポインタを取得
        Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

        if (instance != NULL) {
            // ワールド座標系に対するデバイス座標系の拡大率を更新
            instance->scale += static_cast<GLfloat>(y);
        }
    }

    // キーボード操作時の処理
    static void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
        // このインスタンスのポインタを取得
        Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

        if (instance != NULL) {
            // キー状態を保持
            instance->keyStatus = action;
        }
    }

    // ウィンドウサイズを取り出す
    const GLfloat *getSize() const { return size; }

    // ワールド座標系に対するデバイス座標系の拡大率を取り出す
    GLfloat getScale() const { return scale; }

    // モデル位置を取り出す
    const GLfloat *getModelLoc() const { return modelLoc; }

    // マウス位置を取り出す
    const GLfloat *getMouseLoc() const { return mouseLoc; }
};