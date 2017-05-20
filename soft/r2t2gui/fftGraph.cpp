#include "fftGraph.h"

#include <QGraphicsScene>
#include <QApplication>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <math.h>
#include <stdlib.h>
#include "lib.h"
#include <QTime>
#include "vertex.h"

using namespace std;

// Front Verticies
#define VERTEX_FTR Vertex( QVector3D( 0.5f,  0.5f,  0.5f), QVector3D( 1.0f, 0.0f, 0.0f ) )
#define VERTEX_FTL Vertex( QVector3D(-0.5f,  0.5f,  0.5f), QVector3D( 0.0f, 1.0f, 0.0f ) )
#define VERTEX_FBL Vertex( QVector3D(-0.5f, -0.5f,  0.5f), QVector3D( 0.0f, 0.0f, 1.0f ) )
#define VERTEX_FBR Vertex( QVector3D( 0.5f, -0.5f,  0.5f), QVector3D( 0.0f, 0.0f, 0.0f ) )

// Shader sources
const char* vertexSource = R"glsl(
                             #version 330 core
                             in vec2 position;
                             in vec3 color;
                             in vec2 texcoord;
                             out vec3 Color;
                             out vec2 Texcoord;
                             void main()
                             {
                             Color = color;
                             Texcoord = texcoord;
                             gl_Position = vec4(position, 0.0, 1.0);
                             }
                             )glsl";
const char* fragmentSource = R"glsl(
                               #version 330 core
                               in vec3 Color;
                               in vec2 Texcoord;
                               out vec4 outColor;
                               uniform sampler2D tex;
                               void main()
                               {
//                                                              outColor = texture(tex, Texcoord);
                               outColor = texture(tex, Texcoord) * vec4(Color, 1.0);
//                                 outColor = texture(tex, Texcoord).rgb;
//                                outColor = vec4(Color, 1.0);
                               }
                               )glsl";

FFTGraph::FFTGraph(QSettings *settings, int x, int y, QWidget *parent) : settings(settings), QOpenGLWidget(parent),  xSize(x), ySize(y){
    int i;

    //    setFlags(QGraphicsItem::ItemClipsToShape);

    //    this->setAcceptHoverEvents(false);
    //    this->setAcceptedMouseButtons(Qt::NoButton);
    //    this->setAcceptTouchEvents(false);

    displayMode = GRAPH_WATERFALL;
    nFFT = FFT_SIZE;
    xViewPos = (nFFT-xSize)/2;
    fftPixmap = new QPixmap(nFFT, ySize);
    fftPixmap->fill(QColor(getSettings(settings,"display/colorFFTBackground","#000000")));
    waterfallPixmap = new QPixmap(nFFT, ySize);
    waterfallPixmap->fill(QColor(getSettings(settings,"display/colorFFTBackground","#000000")));
    mi = -1.5;
    tmi = mi;
    ma = -0.5;
    tma = ma;

    setGeometry(0, 0, 512, 512);

    for(i=0; i < MAX_FFT; i++) {
        fftmax[i]=0;
        fftav[i]=0;
    }

    base = 3;
    scale = 40;
    setAuto = false;

    qDebug() << "nFFT" << nFFT;
    qDebug() << "ySize" << ySize;
    fftData = new char[512*512];
    int j = 0;
    for (int i=0; i< 512*512; i++){
        fftData[i] = j;
        j++;
        if (j == 128){
            j = 0;
        }
    }
    qDebug() << "fftSize" << sizeof(fftData);
}

// Create a colored triangle
static const Vertex sg_vertexes[] = {
    //Position + Color
    VERTEX_FTR, VERTEX_FTL, VERTEX_FBL,
    VERTEX_FBL, VERTEX_FBR, VERTEX_FTR,
};

#undef VERTEX_FBR
#undef VERTEX_FBL
#undef VERTEX_FTL
#undef VERTEX_FTR


void FFTGraph::initializeGL(){
    // Initialize OpenGL Backend
    initializeOpenGLFunctions();
//    printVersionInformation();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Create Shader (Do not release until VAO is created)
    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/simple.vert");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/simple.frag");
    m_program->link();
    m_program->bind();

     // Create Buffer (Do not release until VAO is created)
    m_vertex.create();
    m_vertex.bind();
    m_vertex.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertex.allocate(sg_vertexes, sizeof(sg_vertexes));

    // Create Vertex Array Object
    m_object.create();
    m_object.bind();
    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(0, GL_FLOAT, Vertex::positionOffset(), Vertex::PositionTupleSize, Vertex::stride());
    m_program->setAttributeBuffer(1, GL_FLOAT, Vertex::colorOffset(), Vertex::ColorTupleSize, Vertex::stride());

    m_texture.create();


    m_object.release();
    m_vertex.release();
    m_program->release();

//    // Create Vertex Array Object
//    GLuint vao;
//    f->glGenVertexArrays(1, &vao);
//    f->glBindVertexArray(vao);

//    // Create a Vertex Buffer Object and copy the vertex data to it
//    GLuint vbo;
//    f->glGenBuffers(1, &vbo);

////    GLfloat vertices[] = {
////        //  Position      Color             Texcoords
////        -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
////        1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
////        1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
////        -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
////    };

//    GLfloat vertices[] = {
//        //  Position      Color             Texcoords
//        -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // Top-left
//        0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // Top-right
//        0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
//        -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
//    };


//    f->glBindBuffer(GL_ARRAY_BUFFER, vbo);
//    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

//    // Create an element array
//    GLuint ebo;
//    f->glGenBuffers(1, &ebo);

//    GLuint elements[] = {
//        0, 1, 2,
//        2, 3, 0
//    };

//    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

//    // Create and compile the vertex shader
//    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    f->glShaderSource(vertexShader, 1, &vertexSource, NULL);
//    f->glCompileShader(vertexShader);

//    // Create and compile the fragment shader
//    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    f->glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
//    f->glCompileShader(fragmentShader);

//    // Link the vertex and fragment shader into a shader program
//    GLuint shaderProgram = glCreateProgram();
//    f->glAttachShader(shaderProgram, vertexShader);
//    f->glAttachShader(shaderProgram, fragmentShader);
//    f->glBindFragDataLocation(shaderProgram, 0, "outColor");
//    f->glLinkProgram(shaderProgram);
//    f->glUseProgram(shaderProgram);



//    // Specify the layout of the vertex data
//    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
//    f->glEnableVertexAttribArray(posAttrib);
//    f->glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

//    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
//    f->glEnableVertexAttribArray(colAttrib);
//    f->glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

//    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
//    f->glEnableVertexAttribArray(texAttrib);
//    f->glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

//    // Load texture
//    GLuint tex;
//    f->glGenTextures(1, &tex);

//    float pixels[] = {
//        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
//    };

//    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, fftData);


//    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
//    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

//    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

//    fftupdate = new char[256*256];
//    int j = 200;
//    for (int i=0; i< 256*256; i++){
//        fftData[i] = j;
//    }
//        f->glBindTexture(GL_TEXTURE_2D, tex);
//        f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RED, GL_UNSIGNED_BYTE, fftupdate);
}

void FFTGraph::resizeGL(int w, int h)
{
    qDebug("resize to width %d, height %d", w, h);
//    f->glViewport(0,0,w,h);
}

void FFTGraph::paintGL()
{
    // Clear the screen to black
    glClear(GL_COLOR_BUFFER_BIT);
    // Draw a rectangle from the 2 triangles using 6 indices
//    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_program->bind();
    {
        m_object.bind();
        glDrawArrays(GL_TRIANGLES, 0, sizeof(sg_vertexes) / sizeof(sg_vertexes[0]));
        m_object.release();
    }
    m_program->release();
}

FFTGraph::~FFTGraph() {
    delete fftPixmap;
    delete waterfallPixmap;
}

void FFTGraph::setSize(int x, int y) {
    xSize = x;
    ySize = y;
    xViewPos = (nFFT-xSize)/2;
    delete fftPixmap;
    fftPixmap = new QPixmap(nFFT, ySize);
    fftPixmap->fill(QColor(getSettings(settings,"display/colorFFTBackground","#000000")));
    delete waterfallPixmap;
    waterfallPixmap = new QPixmap(nFFT, ySize);
    waterfallPixmap->fill(QColor(getSettings(settings,"display/colorFFTBackground","#000000")));

    //    fftData = new unsigned char[nFFT*ySize]();
    //    qDebug() << "fftdata len" << sizeof(fftData);
}

void FFTGraph::settingsChanged(int mode) {
    double h,s,v;
    average_time_max = getSettings(settings,"display/averageTimeMax",64);
    average_time =     getSettings(settings,"display/averageTime",16);
    colorSpecMax =     QColor(getSettings(settings,"display/colorFFTMax","#d11346"));
    colorSpecAverage = QColor(getSettings(settings,"display/colorFFTAverage","#09f088"));
    colorSpecBack =    QColor(getSettings(settings,"display/colorFFTBackground","#000000"));
    colorSpecMinMax =  QColor(getSettings(settings,"display/colorFFTMinMax","#d0d000"));
    fft_scale =  2;

    double h_min = 1;
    double h_max = 0;
    double s_min = 1;
    double s_max = 1;
    double v_min = 0;
    double v_max = 1;

    for( int i=0; i<256; i++) {
        switch (mode) {
        case 0:
            colorTab[i].setRgb(i,i,i);
            break;
        case 1:
            if( (i<43) )             colorTab[i].setRgb( 0,0, 255*(i)/43);
            if( (i>=43) && (i<87) )  colorTab[i].setRgb( 0, 255*(i-43)/43, 255);
            if( (i>=87) && (i<120))  colorTab[i].setRgb( 0,255, 255-(255*(i-87)/32));
            if( (i>=120) && (i<154)) colorTab[i].setRgb( (255*(i-120)/33), 255, 0);
            if( (i>=154) && (i<217)) colorTab[i].setRgb( 255, 255 - (255*(i-154)/62), 0);
            if( (i>=217))            colorTab[i].setRgb( 255, 0, 128*(i-217)/38);
            break;
        case 2:
            h = (h_max-h_min)*i/256+h_min;
            if (h>1) h-=1;
            s = (s_max-s_min)*i/256+s_min;
            if (s>1) s-=1;
            v = (v_max-v_min)*i/256+v_min;
            if (v>1) v-=1;
            colorTab[i].setHsvF(h,s,v);
            break;
        }
    }
}


//void FFTGraph::paint (QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
////    qDebug() << "drawing FFT/Waterfall";

//    switch (displayMode) {
//        case GRAPH_WATERFALL:
//            painter->drawPixmap(-xViewPos,0,*waterfallPixmap);
//            break;
//        case GRAPH_FFT:
//            painter->drawPixmap(-xViewPos,0,*fftPixmap);
//            break;
//        case GRAPH_DUAL:
//        case GRAPH_DUAL2:
//            painter->drawPixmap(-xViewPos,0,*fftPixmap);
//            painter->drawPixmap(-xViewPos,130,*waterfallPixmap);
//            break;
//    }

//}

void FFTGraph::setFFTSize(int size) {
    nFFT = size;
    xViewPos = (nFFT-xSize)/2;
    delete fftPixmap;
    fftPixmap = new QPixmap(nFFT, ySize);
    fftPixmap->fill(Qt::black);
    delete waterfallPixmap;
    waterfallPixmap = new QPixmap(nFFT, ySize);
    waterfallPixmap->fill(Qt::black);
    for(int i=0; i < MAX_FFT; i++) {
        fftmax[i]=0;
        fftav[i]=0;
    }
}

QRectF FFTGraph::boundingRect() const {
    return QRectF(0, 0, xSize, ySize);
}

void FFTGraph::setMin(int v) {	// dbm
    tmi = v;
    if (tmi>=tma) tma = tmi + 0.1;
    base = tmi;
    scale = 256/(tma-tmi);
}

void FFTGraph::setMax(int v) {   // dbm
    tma = v;
    if (tmi>=tma) tmi = tma - 0.1;
    base = tmi;
    scale = 256/(tma-tmi);
}

void FFTGraph::setDisplayMode(int m) {
    for(int i=0; i < MAX_FFT; i++) {
        fftmax[i]=-10;
        fftav[i]=0;
    }
    fftPixmap->fill(Qt::black);
    waterfallPixmap->fill(Qt::black);
    displayMode = m;
}

void FFTGraph::fftDataReady(QByteArray &data) {



    //    int i;
    //    double v;

    //    if (data.length() == 0) {
    //        return;
    //    }

    //    mi = 0;
    //    ma = -100;

    //    for(i=0; i < nFFT; i++) {
    //        v = -(uint8_t)data[i];
    //        v1[i] = v;
    //        if (fftmax[i]>v)
    //            fftmax[i] = v;
    //        fftmax[i] = ((average_time_max-1)*fftmax[i]+v)/average_time_max;
    //        if (fftav[i]==0)
    //            fftav[i] = v;
    //        else
    //            fftav[i] = ((average_time-1)*fftav[i]+v)/average_time;

    //        if (i>nFFT/8 && i<nFFT*7/8)
    //            mi = min(fftav[i],mi);
    //        ma = max(fftav[i],ma);
    //    }

    //    switch (displayMode) {
    //    case GRAPH_WATERFALL:
    //    {
    //        QPainter painter(waterfallPixmap);
    //        waterfallPixmap->scroll(0,1,waterfallPixmap->rect());
    //        for(i=0; i < nFFT; i++) {
    //            v = (v1[i]-base)*scale;
    //            if (v>255) v=255;
    //            if (v<0)   v=0;
    //            painter.setPen(colorTab[(unsigned char)v]);
    //            painter.drawPoint(i,0);
    //        }
    //        if(setAuto) {
    //            base = -mi;
    //            scale = 256/(ma-mi);
    //            setAuto = false;
    //        }
    //        //                qDebug() << "Waterfall Pixmap size:" << waterfallPixmap->size();
    //    }
    //        break;
    //    case GRAPH_FFT:
    //        fft_scale = 2;
    //    {
    //        QPainter painter(fftPixmap);
    //        fftPixmap->fill(colorSpecBack);
    //        // setPen kostet rechenzeit, daher jede Schleife einzeln
    //        painter.setPen(colorSpecMax);
    //        for(i=1; i < nFFT; i++)
    //            painter.drawLine(i-1,-fftmax[i-1]*fft_scale,i,-fftmax[i]*fft_scale);
    //        painter.setPen(colorSpecAverage);
    //        for(i=1; i < nFFT; i++)
    //            painter.drawLine(i-1,-fftav[i-1]*fft_scale,i,-fftav[i]*fft_scale);
    //        painter.setPen(colorSpecMinMax);
    //        painter.drawLine(0,-mi*fft_scale,nFFT,-mi*fft_scale);
    //        painter.drawLine(0,-ma*fft_scale,nFFT,-ma*fft_scale);
    //    }
    //        break;
    //    case GRAPH_DUAL:
    //        fft_scale = 1;
    //    {
    //        QPainter painter(waterfallPixmap);
    //        waterfallPixmap->scroll(0,1,waterfallPixmap->rect());
    //        for(i=0; i < nFFT; i++) {
    //            v = (v1[i]-base)*scale;
    //            if (v>255) v=255;
    //            if (v<0)   v=0;
    //            painter.setPen(colorTab[(unsigned char)v]);
    //            painter.drawPoint(i,0);
    //        }
    //        if(setAuto) {
    //            base = -mi;
    //            scale = 256/(ma-mi);
    //            setAuto = false;
    //        }
    //    }
    //    {
    //        QPainter painter(fftPixmap);
    //        fftPixmap->fill(colorSpecBack);
    //        painter.setPen(colorSpecMax);
    //        for(i=1; i < nFFT; i++)
    //            painter.drawLine(i-1,-fftmax[i-1]*fft_scale-DUAL_OFFSET*fft_scale,i,-fftmax[i]*fft_scale-DUAL_OFFSET*fft_scale);
    //        painter.setPen(colorSpecAverage);
    //        for(i=1; i < nFFT; i++)
    //            painter.drawLine(i-1,-fftav[i-1]*fft_scale-DUAL_OFFSET*fft_scale,i,-fftav[i]*fft_scale-DUAL_OFFSET*fft_scale);
    //    }
    //        break;
    //    case GRAPH_DUAL2:
    //        fft_scale = 1;
    //    {
    //        QPainter painter(waterfallPixmap);
    //        waterfallPixmap->scroll(0,1,waterfallPixmap->rect());
    //        for(i=0; i < nFFT; i++) {
    //            v = (v1[i]-base)*scale;
    //            if (v>255) v=255;
    //            if (v<0)   v=0;
    //            painter.setPen(colorTab[(unsigned char)v]);
    //            painter.drawPoint(i,0);
    //        }
    //        if(setAuto) {
    //            base = -mi;
    //            scale = 256/(ma-mi);
    //            setAuto = false;
    //        }
    //    }
    //    {
    //        QPainter painter(fftPixmap);
    //        fftPixmap->fill(colorSpecBack);
    //        painter.setPen(colorSpecMax);
    //        for(i=1; i < nFFT; i++) {
    //            v = (v1[i]-base)*scale;
    //            if (v>255) v=255;
    //            if (v<0)   v=0;
    //            painter.setPen(colorTab[(unsigned char)v]);
    //            painter.drawLine(i,-fftmax[i]*fft_scale-DUAL_OFFSET*fft_scale,i,300);
    //        }
    //        painter.setPen(colorSpecMinMax);
    //    }
    //        break;
    //    default:
    //        ;
    //    }
    //    update();
}


void FFTGraph::setAutomaticCB() {
    setAuto = false;
}

int FFTGraph::getMin() {
    return mi;
}

int FFTGraph::getMax() {
    return ma;
}

void FFTGraph::scrollVert(int diff) {
    //    waterfallPixmap->scroll(diff,0,waterfallPixmap->rect());
    //    QPainter painter(waterfallPixmap);
    //    painter.setPen(colorSpecBack);
    //    painter.setBrush(colorSpecBack);
    //    if (diff>0)
    //        painter.drawRect(0,0,diff,ySize);
    //    else
    //        painter.drawRect(nFFT+diff,0,nFFT,ySize);
}
