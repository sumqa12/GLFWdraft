#include <windows.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <random>
#include <GL/glew.h>
#include <GL/glfw3.h>
#include "lib/Matrix"

using GLchar = char;
using namespace std;
// シェーダオブジェクトのコンパイル結果を表示する
//  shader: シェーダオブジェクト名
//  str: コンパイルエラーが発生した場所を表す文字列
GLboolean printShaderInfoLog(GLuint shader, const char *str)
{
    // コンパイル結果を取得
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    printf("shader status: %s\n", status == GL_TRUE ? "Success" : "Failed");
    if (status == GL_FALSE)
    {
        printf("Error : Compile Error in %s\n", str);
    }

    // シェーダのコンパイル時のログの長さを取得
    GLsizei bufSize;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &bufSize);

    if (bufSize > 1)
    {
        // シェーダのコンパイル時のログ内容を取得
        std::vector<GLchar> infoLog(bufSize);
        GLsizei length;
        glGetShaderInfoLog(shader, bufSize, &length, &infoLog[0]);
        printf("printShader: %s", &infoLog[0]);
    }

    return static_cast<GLboolean>(status);
}

// プログラムオブジェクトのリンク結果を表示する
//  program: プログラムオブジェクト名
GLboolean printProgramInfoLog(GLuint program)
{
    // リンク結果を取得
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    printf("shader status: %s\n", status == GL_TRUE ? "Success" : "Failed");
    if (status == GL_FALSE)
    {
        printf("Error : Link Error.\n");
    }

    // シェーダのリンク時のログの長さの取得
    GLsizei bufSize;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufSize);

    if (bufSize > 1)
    {
        // シェーダのリンク時のログの内容の取得
        std::vector<GLchar> infoLog(bufSize);
        GLsizei length;
        glGetProgramInfoLog(program, bufSize, &length, &infoLog[0]);
        printf("printProgram: %s", &infoLog[0]);
    }

    return static_cast<GLboolean>(status);
}

// プログラムオブジェクトの作成
//  vsrc: バーテックスシェーダのソースプログラムの文字列
//  fsrc: フラグメントシェーダのソースプログラムの文字列
GLuint createProgram(const char *vsrc, const char *fsrc)
{
    // 空のプログラムオブジェクトの作成
    const GLuint program(glCreateProgram());

    if (vsrc != NULL)
    {
        // バーテックスシェーダのシェーダオブジェクトの作成
        const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
        glShaderSource(vobj, 1, &vsrc, NULL);
        glCompileShader(vobj);

        // バーテックスシェーダのコンパイル結果を確認
        if (printShaderInfoLog(vobj, "Vertex Shader"))
        {
            glAttachShader(program, vobj);
            printf("Attached vertex shader\n");
        }
        else
        {
            printf("Vertex shader compilation failed\n");
        }
        glDeleteShader(vobj);
    }
    else
    {
        printf("vsrc is null\n");
    }

    if (fsrc != NULL)
    {
        // フラグメントシェーダのシェーダオブジェクトの作成
        const GLuint fobj(glCreateShader(GL_FRAGMENT_SHADER));
        glShaderSource(fobj, 1, &fsrc, NULL);
        glCompileShader(fobj);

        // フラグメントシェーダのコンパイル結果を確認
        if (printShaderInfoLog(fobj, "Fragment Shader"))
        {
            glAttachShader(program, fobj);
            printf("Attached fragment shader\n");
        }
        else
        {
            printf("Fragment shader compilation failed\n");
        }
        glDeleteShader(fobj);
    }
    else
    {
        printf("fsrc is null\n");
    }

    // プログラムオブジェクトをリンクする
    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "normal");
    glBindFragDataLocation(program, 0, "fragment");
    glLinkProgram(program);

    // 作成したプログラムオブジェクトを返す
    if (printProgramInfoLog(program))
    {
        return program;
    }

    // 失敗したら 0 を返す
    glDeleteProgram(program);
    return 0;
}

// シェーダのソースファイルを読み込んだメモリを返す
//  name: シェーダのソースファイル名
//  buffer: 読み込んだよースファイルのテキスト
vector<GLchar> readShaderSource(const char *name, std::vector<GLchar> &buffer)
{
    vector<GLchar> nullVec;

    // ファイル名がNULL
    if (name == NULL)
        return nullVec;

    printf("Trying to open source file: %s\n", name); // デバッグメッセージを追加

    // ソースファイルを開く
    std::ifstream file(name, std::ios::binary);

    if (!file.is_open())
    {
        printf("Error : Can't open source file: %s\n", name);
        return nullVec;
    }

    // ファイルの末尾に移動し現在位置(=ファイルサイズ)を得る
    file.seekg(0L, std::ios::end);
    long long int length = file.tellg();
    file.seekg(0L, std::ios::beg);

    printf("File length: %lld\n", length); // ファイルサイズを出力

    if (length <= 0)
    {
        printf("Error: File is empty or could not determine length.\n");
        return nullVec;
    }

    // データをchar型に (リサイズ)
    std::vector<GLchar> data(length + 1); // std::vectorを使用してメモリ管理を簡素化

    // ファイル読み込み
    file.read(data.data(), length);

    if (file.fail())
    { // 読み込み失敗をチェック
        printf("Error : Could not read source file.\n");
        return nullVec;
    }

    // NULL終端を追加
    data[length] = '\0';

    printf("Done : File read.\n");

    return data;
}

// シェーダのソースファイルを読み込んでプロクラムオブジェクトを作成する
//  vert: バーテックスシェーダのソースファイル名
//  frag: フラグメントシェーダのソースファイル名
GLuint loadProgram(const char *vert, const char *frag)
{

    // シェーダのソースファイルを読み込む
    std::vector<GLchar> vsrc = readShaderSource(vert, vsrc);
    std::vector<GLchar> fsrc = readShaderSource(frag, fsrc);

    // 読み込んだ内容を出力
    printf("Vertex File content: %s\n", vsrc.data());
    printf("Fragment File content: %s\n", fsrc.data());

    // プログラムオブジェクトを作成
    printf("create program obj\n");
    return vsrc.data() != nullptr && fsrc.data() != nullptr ? createProgram(vsrc.data(), fsrc.data()) : 0;
}

// 矩形の頂点の位置
constexpr Object::Vertex rectangleVertex1[] = {
    {-1, -1, 0},
    {1, -1, 0},
    {1, 1, 0},
    {-1, 1, 0}
};
constexpr Object::Vertex rectangleVertex2[] = {
    {-0.5f, -0.5f, 0},
    {0.5, -0.5f, 0},
    {0.5f, 0.5f, 0},
    {-0.5f, 0.5f, 0}
};
constexpr Object::Vertex dot[] = {
    {-0.01f, -0.01f, 0},
    {0.01f, -0.01f, 0},
    {0.01f, 0.01f, 0},
    {-0.01f, 0.01f, 0}
};
// 八面体の頂点の位置 
constexpr Object::Vertex octahedronVertex[] = { 
  {  0.0f,  1.0f,  0.0f }, 
  { -1.0f,  0.0f,  0.0f }, 
  {  0.0f, -1.0f,  0.0f }, 
  {  1.0f,  0.0f,  0.0f }, 
  {  0.0f,  1.0f,  0.0f }, 
  {  0.0f,  0.0f,  1.0f }, 
  {  0.0f, -1.0f,  0.0f }, 
  {  0.0f,  0.0f, -1.0f }, 
  { -1.0f,  0.0f,  0.0f }, 
  {  0.0f,  0.0f,  1.0f }, 
  {  1.0f,  0.0f,  0.0f }, 
  {  0.0f,  0.0f, -1.0f } 
};
// 六面体の頂点の位置と色 
constexpr Object::Vertex cubeVertex[] = 
{ 
  { -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f },  // (0) 
  { -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.8f },  // (1) 
  { -1.0f,  1.0f,  1.0f,  0.0f,  0.8f,  0.0f },  // (2) 
  { -1.0f,  1.0f, -1.0f,  0.0f,  0.8f,  0.8f },  // (3) 
  {  1.0f,  1.0f, -1.0f,  0.8f,  0.0f,  0.0f },  // (4) 
  {  1.0f, -1.0f, -1.0f,  0.8f,  0.0f,  0.8f },  // (5) 
  {  1.0f, -1.0f,  1.0f,  0.8f,  0.8f,  0.0f },  // (6) 
  {  1.0f,  1.0f,  1.0f,  0.8f,  0.8f,  0.8f }   // (7) 
};
// 六面体の稜線の両端点のインデックス 
constexpr GLuint wireCubeIndex[] = { 
  1, 0, // (a) 
  2, 7, // (b) 
  3, 0, // (c) 
  4, 7, // (d) 
  5, 0, // (e) 
  6, 7, // (f) 
  1, 2, // (g) 
  2, 3, // (h) 
  3, 4, // (i) 
  4, 5, // (j) 
  5, 6, // (k) 
  6, 1  // (l) 
};
// 面ごとに法線を変えた六面体の頂点属性 
constexpr Object::Vertex solidCubeVertex[] = 
{ 
  // 左 
  { -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f }, 
  { -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f }, 
  { -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f }, 
  { -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f }, 
  { -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f }, 
  { -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f }, 
 
  // 裏 
  {  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f }, 
  { -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f }, 
  { -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f }, 
  {  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f }, 
  { -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f }, 
  {  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f }, 
 
  // 下 
  { -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f }, 
  {  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f }, 
  {  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f }, 
  { -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f }, 
  {  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f }, 
  { -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f }, 
 
  // 右 
  {  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f }, 
  {  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f }, 
  {  1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f }, 
  {  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f }, 
  {  1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f }, 
  {  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f }, 
 
  // 上 
  { -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f }, 
  { -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f }, 
  {  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f }, 
  { -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f }, 
  {  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f }, 
  {  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f }, 
 
  // 前 
  { -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f }, 
  {  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f }, 
  {  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f }, 
  { -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f }, 
  {  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f }, 
  { -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f } 
};

// 六面体の面を塗りつぶす三角形の頂点のインデックス 
constexpr GLuint solidCubeIndex[] = {
     0,  1,  2,  3,  4,  5, // 左 
     6,  7,  8,  9, 10, 11, // 裏 
    12, 13, 14, 15, 16, 17, // 下 
    18, 19, 20, 21, 22, 23, // 右 
    24, 25, 26, 27, 28, 29, // 上 
    30, 31, 32, 33, 34, 35  // 前
};

int main()
{

    char cdir[255];
    GetCurrentDirectory(255, cdir);
    printf("current_path: %s\n", cdir);

    // GLFWの初期化
    if (glfwInit() != GLFW_TRUE)
    {
        printf("Error : Can't initialize GLFW.\n");
        return 1;
    }

    // プログラム終時の処理の登録
    atexit(glfwTerminate);

    // OpenGL Version 4.6 Core Profile を選択
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // デフォルト設定
    //  glfwDefaultWindowHints();

    // ウィンドウ作成
    Window window;

    // 背景色を指定
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    // 背面カリングを有効
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // デプスバッファ
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    // プログラムオブジェクトの作成
    const GLuint program = loadProgram("../point.vert", "../point.frag");
    if (program == 0)
    {
        printf("Error: Could not loadProgram.\n");
        return 1;
    }

    // uniform変数の場所を取得
    const GLint modelviewLoc(glGetUniformLocation(program, "modelview"));
    const GLint projectionLoc(glGetUniformLocation(program, "projection"));
    const GLint normalMatrixLoc(glGetUniformLocation(program, "normalMatrix"));
    const GLint LposLoc(glGetUniformLocation(program, "Lpos"));
    const GLint LambLoc(glGetUniformLocation(program, "Lamb"));
    const GLint LdiffLoc(glGetUniformLocation(program, "Ldiff"));
    const GLint LspecLoc(glGetUniformLocation(program, "Lspec"));

    // 球の分割数
    const int slices(32), stacks(16);

    // 頂点属性を作る
    std::vector<Object::Vertex> solidSphereVertex;
    for (int j = 0; j <= stacks; j++) {
        const float t(static_cast<float>(j) / static_cast<float>(stacks));
        const float y(cos(3.121593f * t)), r(sin(3.141593f * t));
        
        for (int i = 0; i <= slices; i++) {
            const float s(static_cast<float>(i) / static_cast<float>(slices));
            const float z(r * cos(6.283185f * s)), x(r * sin(6.283185 * s));

            // 頂点属性
            const Object::Vertex v = {x, y, z, x, y, z};

            // 頂点属性を追加
            solidSphereVertex.emplace_back(v);
        }
    }

    // インデックスを作る
    std::vector<GLuint> solidSphereIndex;

    for (int j = 0; j < stacks; j++) { 
        const int k((slices + 1) * j); 
    
        for (int i = 0; i < slices; i++) { 
            // 頂点のインデックス 
            const GLuint k0(k + i); 
            const GLuint k1(k0 + 1); 
            const GLuint k2(k1 + slices); 
            const GLuint k3(k2 + 1);

             // 左下の三角形 
            solidSphereIndex.emplace_back(k0); 
            solidSphereIndex.emplace_back(k2); 
            solidSphereIndex.emplace_back(k3); 
        
            // 右上の三角形 
            solidSphereIndex.emplace_back(k0); 
            solidSphereIndex.emplace_back(k3); 
            solidSphereIndex.emplace_back(k1); 
        }
    }

    // 図形データを作成する
    std::unique_ptr<const Shape> shape(new SolidShapeIndex(3,
        static_cast<GLsizei>(solidSphereVertex.size()), solidSphereVertex.data(),
        static_cast<GLsizei>(solidSphereIndex.size()), solidSphereIndex.data())
    );

    // 光源データ
    GLfloat r = 0, g = 0, b = 0;
    static constexpr Vector Lpos = {0.0f, 0.0f, 2.4f, 1.0f};
    static constexpr GLfloat Lamb[] = {0.2f, 0.1f, 0.1f};
    static  GLfloat Ldiff[] = {1, 1, 1};
    static constexpr GLfloat Lspec[] = {1.0f, 0.5f, 0.5f};

    // タイマーを0にセット
    glfwSetTime(0.0);
    

    printf("OpenGL ver.: %s\n", glGetString(GL_VERSION));
    printf("GLSL ver.: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));


    std::mt19937 engine{std::random_device{}()};
    std::uniform_real_distribution<float> dist(0, 1);
    
    GLfloat lg = 0;
    bool lf = false;
    bool rgb = false;
    // ウィンドウが開いている間
    while (window)
    {
        if (r >= 1) rgb = true;
        else if (r <= 0)rgb = false;
        if (rgb) r -= 0.01f;
        else r += 0.01f;
        // Ldiff[0] = r;
        // Ldiff[1] = r;
        // Ldiff[2] = r;
        

        if (lg >= 180.0f) lg = 0;
        else lg += 0.1f;

        // ウィンドウを消去
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // シェーダプログラムの使用開始
        glUseProgram(program);


        // 透視投影変換行列を求める 
        const GLfloat *const size(window.getSize()); 
        const GLfloat fovy(window.getScale() * 0.01f);
        const GLfloat aspect(size[0] / size[1]);
        const Matrix projection(Matrix::perspective(fovy, aspect, 1.0f, 10.0f));

        // モデル変換行列を求める
        const GLfloat *const modelLoc(window.getModelLoc());
        const GLfloat *const mouseLoc(window.getMouseLoc());

        printf("x, y: %.2f, %.2f\n", mouseLoc[0], mouseLoc[1]);

        const Matrix rx(Matrix::rotateAxis(mouseLoc[0] * 2, 0.0f, 1.0f, 0.0f));
        const Matrix ry(Matrix::rotateAxis(mouseLoc[1] * 2, 1.0f, 0.0f, 0.0f));
        const Matrix model(Matrix::translate(modelLoc[0] * 2, modelLoc[1] * 2, 0.0f) * ry * rx);
        
        // ビュー変換行列を求める
        const Matrix view(Matrix::lookat(3.0f, 4.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)); 

        // 法線ベクトルの変換行列の格納先
        GLfloat normalMatrix[9];

        // モデルビュー変換行列を求める
        const Matrix modelview(view * model);

        // 法線ベクトルの変換行列を求める
        modelview.getNormalMatrix(normalMatrix);

        // uniform 変数に値を設定する 
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection.data());
        glUniformMatrix4fv(modelviewLoc, 1, GL_FALSE, modelview.data()); 
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, normalMatrix);
        glUniform4fv(LposLoc, 1 , (view * Lpos).data());
        glUniform3fv(LambLoc, 1 , Lamb);
        glUniform3fv(LdiffLoc, 1 , Ldiff);
        glUniform3fv(LspecLoc, 1 , Lspec);

        // 図形の描画
        shape->draw();

        // 二つ目のモデルビュー変換行列を求める
        const Matrix modelview1(modelview * Matrix::translate(0.0f, 0.0f, 3.0f));
        
        // 二つ目の法線ベクトルの変換行列を求める
        modelview1.getNormalMatrix(normalMatrix);

        // uniform変数に値を設定
        glUniformMatrix4fv(modelviewLoc, 1, GL_FALSE, modelview1.data());
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, normalMatrix);

        // 二つ目の図形の描画
        shape->draw();

        // カラーバッファを入れ替え
        window.swapBuffers();
    }
}