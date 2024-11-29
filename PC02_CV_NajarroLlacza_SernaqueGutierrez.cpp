/*
Referencias:
Código base que utilizamos para empezar el programa, algoritmo de bezier, cabe mencionar que esta curva lo implementamos de modo adicional: https://github.com/kodiidok/glut-bezier-curves
Información adicional de algunas funciones en OpenGL: https://www.fing.edu.uy/inco/cursos/compgraf/Clases/2012/09-Mallas%20poligonales%20y%20curvas%5BopenGL%5D.pdf
También usamos el código hecho en clase de la rotación del cubo para la rotación de las curvas 3D

Autores:
Said William Najarro Llacza - 22200031
José Roberto Sernaqué Gutierrez - 22200063

Agradecimientos especiales:
ChatGPT: https://chatgpt.com/ para la lógica del mouse y movimiento de los puntos de control.

Observaciones:
Para compilarlo en Codeblocks se debe utilizar c++ 11 ya que hay un for que no está definido en versiones anteriores
Pasos:
- Ir a la parte superior y seleccionar la opción "Project"
- En el submenú que aparece seleccionar la opción "Build options..."
- Seleccionar la casilla "Have g++ follow the C++11 ISO C++ lenguage standart [-std=c++11]"
- Compilar y ejecutar el programa

Si se quiere usar Visual Code, no hay problema, ya que está actualizado y se puede compilar sin ningún porblema.
*/

#include <stdio.h>
#include <iostream>
#include <cmath>
#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <vector>
#include <locale>

using namespace std;

//Declaración de variables globales
//Variables de tamaño de ventana
int WINDOW_WIDTH = 1440;
int WINDOW_HEIGHT = 480;

float sq_pos[] = { 0.0f, 0.0f };
float sq_rot = 0.0f;
float sq_scl = 1.0f;

//Varaibles para la imagen 3D
float anguloX = 0.0f;
float anguloY = 0.0f;
float anguloZ = 0.0f; // Añadido para rotación en el eje Z
bool showCurveIn3D = true;
bool showCurveIn2D = true;
GLfloat curveColor3D[] = { 0.0f, 0.0f, 0.0f }; // Color de la curva en la segunda gráfica 3D


//Varaibles para habilitar las funciones del menú
bool showPolyline = true;
bool showAxes = true;
bool showControlPoints = true;
bool alternaceCurve = true;
bool addControlPointsEnabled = true;
bool moveControlPointsEnabled = true;

const float pi = 3.14;

int selectedPoint = -1;

GLfloat curveColor[] = { 0.0f, 0.0f, 0.0f }; // Color de la curva cambiado a negro
GLfloat curveColorCinta[] = { 1.0f, 0.5f, 0.3f }; // Color de la curva cambiado a negro
GLfloat bgColor[] = { 1.0f, 1.0f, 1.0f }; // Color de fondo cambiado a blanco

int displayOption = 1;

//Grosor de la línea de las curvas
float lineWidth = 1.0f;

float extrusionDepth = 0.1f; // Profundidad de la extrusión

//Estructura para declarar el punto y es el que se utilizará para instaciar los puntos de control y los de las curvas
struct Point {
    float x, y;
    Point(float _x, float _y) : x(_x), y(_y) {}
};
//Arreglo de puntos prinicpalmente para los puntos de control
vector<Point> controlPoints;
bool curveComplete = false;

//Función que devuelve un punto en la curva de Bezier
Point computeBezierPoint(float t) {
    vector<Point> tempPoints = controlPoints;

    while (tempPoints.size() > 1) {
        vector<Point> newPoints;
        for (size_t i = 0; i < tempPoints.size() - 1; ++i) {
            float x = (1 - t) * tempPoints[i].x + t * tempPoints[i + 1].x;
            float y = (1 - t) * tempPoints[i].y + t * tempPoints[i + 1].y;
            newPoints.push_back(Point(x, y));
        }
        tempPoints = newPoints;
    }

    if (!tempPoints.empty()) {
        return tempPoints.front();
    }
    else {
        return Point(0.0, 0.0);
    }
}

//Función para dibujar los ejes coordenados
void dibujarEjes() {
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(-1.0, 0.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, -1.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, -1.0);
    glVertex3f(0.0, 0.0, 1.0);
    glEnd();
}

//Función para dibujar la plataforma 3D
void dibujarPlataforma() {
    glColor3f(0.9, 0.9, 0.9);
    glBegin(GL_QUADS);
    glVertex3f(-0.9, 0.0, -0.9);
    glVertex3f(0.9, 0.0, -0.9);
    glVertex3f(0.9, 0.0, 0.9);
    glVertex3f(-0.9, 0.0, 0.9);
    glEnd();

    glColor3f(0.2, 0.2, 0.2);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-0.9, 0.001, -0.9);
    glVertex3f(0.9, 0.001, -0.9);
    glVertex3f(0.9, 0.001, 0.9);
    glVertex3f(-0.9, 0.001, 0.9);
    glEnd();
}

//Función para dibujar la curva de Bezier con los puntos generados por la función anterior
void drawBezierCurve() {
    glColor3fv(curveColor);
    glLineWidth(lineWidth);
    glBegin(GL_LINE_STRIP);
    for (float t = 0; t <= 1; t += 0.01) {
        Point p = computeBezierPoint(t);
        glVertex2f(p.x, p.y);
    }
    glEnd();
    glLineWidth(1.0f);
}

//Función que genera la curva de Bezier en el plano xz y el grosor en el plano y
void drawBezierExtrusion3D() {
    glColor3fv(curveColor3D);
    glBegin(GL_QUAD_STRIP);
    for (float t = 0; t <= 1; t += 0.01) {
        Point p = computeBezierPoint(t);
        float x = p.x / 10.0f;
        float y = p.y / 10.0f;
        glVertex3f(x, 0.0f, y);
        glVertex3f(x, 0.0f + extrusionDepth, y);
    }
    glEnd();
}

// Función que genera la cinta de polilíneas con los puntos de control en 3D
void drawBezierExtrusion3DCinta() {
    glColor3fv(curveColorCinta);
    glBegin(GL_QUAD_STRIP);
    for (size_t i = 0; i < controlPoints.size(); ++i) {
        Point p = controlPoints[i];
        float x = p.x / 10.0f;
        float y = p.y / 10.0f;
        glVertex3f(x, 0.0f, y);
        glVertex3f(x, extrusionDepth, y); // Añadir volumen en la dirección Y
    }
    glEnd();
}

//Dibuja los puntos de control generados con el clic
void drawControlPoints() {
    glColor3f(0.0, 1.0, 0.0);
    glPointSize(5.0);
    glBegin(GL_POINTS);
    for (const auto& point : controlPoints) {
        glVertex2f(point.x, point.y);
    }
    glEnd();
    glPointSize(1.0);
}

//Dibuja la líneas que conectan los puntos de control
void drawPolyline() {
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for (const auto& point : controlPoints) {
        glVertex2f(point.x, point.y);
    }
    glEnd();
}

//Dibuja los ejes coordenados 2D
void drawAxes() {
    glColor3f(0.0f, 0.0f, 0.0f); // Cambiar el color de los ejes a negro
    glPointSize(1.0);
    glBegin(GL_LINES);
    glVertex2f(-10.0f, 0.0f);
    glVertex2f(10.0f, 0.0f);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f); // Cambiar el color de los ejes a negro
    glPointSize(1.0);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 10.0f);
    glVertex2f(0.0f, -10.0f);
    glEnd();
}

//Dibuja la curva basada en el algoritmo de BSpline en el plano 2D
void drawBSplineCurve() {
    glColor3fv(curveColor);
    glLineWidth(lineWidth);

    if (controlPoints.size() < 4) return;

    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < controlPoints.size() - 3; ++i) {
        for (float t = 0; t <= 1; t += 0.01) {
            float it = 1.0f - t;
            float b0 = it * it * it / 6.0f;
            float b1 = (3.0f * t * t * t - 6.0f * t * t + 4.0f) / 6.0f;
            float b2 = (-3.0f * t * t * t + 3.0f * t * t + 3.0f * t + 1.0f) / 6.0f;
            float b3 = t * t * t / 6.0f;

            float x = b0 * controlPoints[i].x +
                b1 * controlPoints[i + 1].x +
                b2 * controlPoints[i + 2].x +
                b3 * controlPoints[i + 3].x;
            float y = b0 * controlPoints[i].y +
                b1 * controlPoints[i + 1].y +
                b2 * controlPoints[i + 2].y +
                b3 * controlPoints[i + 3].y;

            glVertex2f(x, y);
        }
    }
    glEnd();
}

//Dibuja la curva basada en el algoritmo de BSpline en el plano 3D, solo se aumenta el grosor
void drawBSplineExtrusion3D() {
    if (controlPoints.size() < 4) {
        std::cerr << "Error: No hay suficientes puntos de control." << std::endl;
        return;
    }

    glColor3fv(curveColor3D);
    glBegin(GL_QUAD_STRIP);
    for (size_t i = 0; i < controlPoints.size() - 3; ++i) {
        for (float t = 0; t <= 1; t += 0.01) {
            float it = 1.0f - t;
            float b0 = it * it * it / 6.0f;
            float b1 = (3.0f * t * t * t - 6.0f * t * t + 4.0f) / 6.0f;
            float b2 = (-3.0f * t * t * t + 3.0f * t * t + 3.0f * t + 1.0f) / 6.0f;
            float b3 = t * t * t / 6.0f;

            float x = b0 * controlPoints[i].x +
                b1 * controlPoints[i + 1].x +
                b2 * controlPoints[i + 2].x +
                b3 * controlPoints[i + 3].x;
            float y = b0 * controlPoints[i].y +
                b1 * controlPoints[i + 1].y +
                b2 * controlPoints[i + 2].y +
                b3 * controlPoints[i + 3].y;

            x /= 10.0f;
            y /= 10.0f;

            glVertex3f(x, 0.0f, y);
            glVertex3f(x, extrusionDepth, y);
        }
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();

    // Configuración de la vista 2D
    glViewport(0, 0, WINDOW_WIDTH / 3, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-11, 11, -11, 11);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Dibujar la curva seleccionada (Bézier o B-Spline)
    if (showCurveIn2D) {
        if (alternaceCurve) {
            drawBezierCurve();
        }
        else {
            drawBSplineCurve();
        }
    }
    // Dibujar la polilínea si está habilitada
    if (showPolyline) {
        drawPolyline();
    }
    // Dibujar los ejes si están habilitados
    if (showAxes) {
        drawAxes();
    }
    // Dibujar los puntos de control si están habilitados
    if (showControlPoints) {
        drawControlPoints();
    }

    // Configuración de la primera vista 3D
    glViewport(2 * WINDOW_WIDTH / 3, 0, WINDOW_WIDTH / 3, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)WINDOW_WIDTH / (3.0 * WINDOW_HEIGHT), 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(2.0, 2.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // Aplicar rotaciones en los ejes X, Y y Z
    glRotatef(anguloX, 1.0, 0.0, 0.0);
    glRotatef(anguloY, 0.0, 1.0, 0.0);
    glRotatef(anguloZ, 0.0, 0.0, 1.0);

    // Dibujar la plataforma y los ejes
    dibujarPlataforma();
    dibujarEjes();

    // Dibujar la extrusión 3D de la curva seleccionada (Bézier o B-Spline) si está habilitada
    if (showCurveIn3D) {
        if (alternaceCurve) {
            drawBezierExtrusion3D();
        }
        else {
            drawBSplineExtrusion3D();
        }
    }

    // Configuración de la segunda vista 3D

    glViewport(WINDOW_WIDTH / 3, 0, WINDOW_WIDTH / 3, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)WINDOW_WIDTH / (3.0 * WINDOW_HEIGHT), 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(2.0, 2.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // Aplicar rotaciones en los ejes X, Y y Z
    glRotatef(anguloX, 1.0, 0.0, 0.0);
    glRotatef(anguloY, 0.0, 1.0, 0.0);
    glRotatef(anguloZ, 0.0, 0.0, 1.0);

    // Dibujar la plataforma y los ejes
    dibujarPlataforma();
    dibujarEjes();

    // Dibujar la extrusión 3D de la curva seleccionada (Bézier o B-Spline) si está habilitada
    if (showCurveIn3D) {
        drawBezierExtrusion3DCinta();
    }



    glPopMatrix();
    glFlush();
}


//Función que reacomoda el display con el tamaño de la pantalla
void reshape(int w, int h) {
    WINDOW_WIDTH = w;
    WINDOW_HEIGHT = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-11, 11, -11, 11);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//Uso del teclado para limpiar y mover la imagen 3D
void keyboard(unsigned char key, int x, int y) {
    if (key == 'c') {
        controlPoints.clear();
        curveComplete = false;
        cout << "Control points cleared. Start a new curve." << endl;
    }
    else if (key == 'q') {
        anguloY += 5.0f; // Rotación en sentido horario alrededor del eje Y
        if (anguloY > 360.0f) anguloY -= 360.0f;
        glutPostRedisplay();
    }
    else if (key == 'w') {
        anguloY -= 5.0f; // Rotación en sentido antihorario alrededor del eje Y
        if (anguloY < 0.0f) anguloY += 360.0f;
        glutPostRedisplay();
    }
    else if (key == 'a') {
        anguloX += 5.0f; // Rotación en sentido horario alrededor del eje X
        if (anguloX > 360.0f) anguloX -= 360.0f;
        glutPostRedisplay();
    }
    else if (key == 's') {
        anguloX -= 5.0f; // Rotación en sentido antihorario alrededor del eje X
        if (anguloX < 0.0f) anguloX += 360.0f;
        glutPostRedisplay();
    }
    else if (key == 'z') {
        anguloZ += 5.0f; // Rotación en sentido horario alrededor del eje Z
        if (anguloZ > 360.0f) anguloZ -= 360.0f;
        glutPostRedisplay();
    }
    else if (key == 'x') {
        anguloZ -= 5.0f; // Rotación en sentido antihorario alrededor del eje Z
        if (anguloZ < 0.0f) anguloZ += 360.0f;
        glutPostRedisplay();
    }
}
void motion(int x, int y) {
    if (selectedPoint >= 0 && moveControlPointsEnabled) {
        int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
        int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

        // Normalizar las coordenadas del mouse
        float normalizedX = (float)x / windowWidth;
        float normalizedY = 1.0f - (float)y / windowHeight;

        // Convertir las coordenadas normalizadas a coordenadas del mundo
        float worldX = -10.0f + 20.0f * (normalizedX / 0.33f); // Ajuste para el rango [-10, 10]
        float worldY = 20.0f * normalizedY - 10.0f;

        // Actualizar la posición del punto de control seleccionado
        controlPoints[selectedPoint] = Point(worldX, worldY);
        glutPostRedisplay(); // Refrescar la pantalla después de mover un punto
    }
}


void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
        int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

        // Normalizar las coordenadas del mouse
        float normalizedX = (float)x / windowWidth;
        float normalizedY = 1.0f - (float)y / windowHeight;

        // Convertir las coordenadas normalizadas a coordenadas del mundo
        float worldX = -10.0f + 20.0f * (normalizedX / 0.33f); // Ajuste para el rango [-10, 10]
        float worldY = 20.0f * normalizedY - 10.0f;

        if (normalizedX <= 0.33f) { // Solo en la primera vista (2D)
            bool puntoSeleccionado = false;

            // Verificar si se ha seleccionado un punto de control existente
            for (size_t i = 0; i < controlPoints.size(); ++i) {
                float dx = controlPoints[i].x - worldX;
                float dy = controlPoints[i].y - worldY;

                if (sqrt(dx * dx + dy * dy) < 1.0f) {
                    if (moveControlPointsEnabled) { // Verificar si mover está habilitado
                        selectedPoint = i;
                        glutMotionFunc(motion); // Solo asignar función de movimiento si está habilitado
                    }
                    puntoSeleccionado = true;
                    break;
                }
            }

            // Si no se seleccionó un punto, agregar un nuevo punto de control
            if (!puntoSeleccionado && addControlPointsEnabled) {
                controlPoints.push_back(Point(worldX, worldY));
            }

            selectedPoint = puntoSeleccionado ? selectedPoint : -1;
            glutPostRedisplay();
        }
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        selectedPoint = -1;
        glutMotionFunc(NULL); // Desactivar movimiento al soltar el clic
    }
}


//Función que opera con la opción seleccionada en el menú
void optionHandler(int option) {
    switch (option) {
    case 1:
        addControlPointsEnabled = !addControlPointsEnabled;
        cout << (addControlPointsEnabled ? "Añadir puntos de control habilitado" : "Añadir puntos de control deshabilitado") << endl;
        break;
    case 2:
        showPolyline = !showPolyline;
        cout << (showPolyline ? "Mostrar polilinea" : "Ocultar polilinea") << endl;
        glutPostRedisplay();
        break;
    case 3:
        showControlPoints = !showControlPoints;
        cout << (showControlPoints ? "Mostrar puntos de control" : "Ocultar puntos de control") << endl;
        glutPostRedisplay();
        break;
    case 4:
        alternaceCurve = !alternaceCurve;
        cout << (alternaceCurve ? "Mostrar Curva de Bezier" : "Mostrar Curva B-Spline") << endl;
        glutPostRedisplay();
        break;
    case 5:
        moveControlPointsEnabled = !moveControlPointsEnabled;
        cout << (moveControlPointsEnabled ? "Mover puntos de control habilitado" : "Mover puntos de control deshabilitado") << endl;
        glutPostRedisplay();
        break;
    case 6:
        showAxes = !showAxes;
        cout << (showAxes ? "Ejes 2D mostrados" : "Ejes 2D ocultos.") << endl;
        glutPostRedisplay();
        break;
    case 71:
        cout << "Cambiar color de la curva en 2D a Rojo" << endl;
        curveColor[0] = 1.0f; // Rojo
        curveColor[1] = 0.0f;
        curveColor[2] = 0.0f;
        glutPostRedisplay();
        break;
    case 72:
        cout << "Cambiar color de la curva en 2D a Verde" << endl;
        curveColor[0] = 0.0f; // Verde
        curveColor[1] = 1.0f;
        curveColor[2] = 0.0f;
        glutPostRedisplay();
        break;
    case 73:
        cout << "Cambiar color de la curva en 2D a Azul" << endl;
        curveColor[0] = 0.0f; // Azul
        curveColor[1] = 0.0f;
        curveColor[2] = 1.0f;
        glutPostRedisplay();
        break;
    case 74:
        cout << "Cambiar color de la curva en 2D a Aleatorio" << endl;
        curveColor[0] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        curveColor[1] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        curveColor[2] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        glutPostRedisplay();
        break;
    case 81:
        lineWidth = 1.0f;
        cout << "Grosor de la linea cambiado a delgado" << endl;
        glutPostRedisplay();
        break;
    case 82:
        lineWidth = 3.0f;
        cout << "Grosor de la linea cambiado a grueso" << endl;
        glutPostRedisplay();
        break;
    case 9:
        cout << "Cambiar el color de la superficie derecha seleccionado" << endl;
        bgColor[0] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        bgColor[1] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        bgColor[2] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.0f);
        glutPostRedisplay();
        break;
    case 101:
        cout << "Cambiar color de la cinta poligonal a opcion 1" << endl;
        curveColorCinta[0] = 1.0f;
        curveColorCinta[1] = 0.0f;
        curveColorCinta[2] = 0.0f;
        glutPostRedisplay();
        break;
    case 102:
        cout << "Cambiar color de la cinta poligonal a opcion 2" << endl;
        curveColorCinta[0] = 0.0f;
        curveColorCinta[1] = 1.0f;
        curveColorCinta[2] = 0.0f;
        glutPostRedisplay();
        break;
    case 103:
        cout << "Cambiar color de la curva 3D a Rojo" << endl;
        curveColor3D[0] = 1.0f;
        curveColor3D[1] = 0.0f;
        curveColor3D[2] = 0.0f;
        glutPostRedisplay();
        break;
    case 104:
        cout << "Cambiar color de la curva 3D a Verde" << endl;
        curveColor3D[0] = 0.0f;
        curveColor3D[1] = 1.0f;
        curveColor3D[2] = 0.0f;
        glutPostRedisplay();
        break;
    case 105:
        cout << "Cambiar color de la curva 3D a Azul" << endl;
        curveColor3D[0] = 0.0f;
        curveColor3D[1] = 0.0f;
        curveColor3D[2] = 1.0f;
        glutPostRedisplay();
        break;
    case 106:
        cout << "Cambiar color de la curva 3D a Aleatorio" << endl;
        curveColor3D[0] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        curveColor3D[1] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        curveColor3D[2] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        glutPostRedisplay();
        break;
    case 11:
        showCurveIn3D = true;
        cout << "Graficar curva en 3D activado" << endl;
        glutPostRedisplay();
        break;
    case 13:
        showCurveIn3D = false;
        cout << "Graficar curva en 3D desactivado" << endl;
        glutPostRedisplay();
        break;
    case 12:
        exit(0);
        break;
    case 30:
        showCurveIn2D = !showCurveIn2D;
        if (showCurveIn2D) {
            cout << "Graficar curva en 2D activado" << endl;
        }
        else {
            cout << "Graficar curva en 2D desactivado" << endl;
        }
        glutPostRedisplay();
        break;
    }
}
void createMenu() {
    int colorSubMenu = glutCreateMenu(optionHandler);
    glutAddMenuEntry("Rojo", 71);
    glutAddMenuEntry("Verde", 72);
    glutAddMenuEntry("Azul", 73);
    glutAddMenuEntry("Aleatorio", 74);

    int grosorSubMenu = glutCreateMenu(optionHandler);
    glutAddMenuEntry("Delgado", 81);
    glutAddMenuEntry("Grueso", 82);

    int cintaPoligonalSubMenu = glutCreateMenu(optionHandler);
    glutAddMenuEntry("Rojo", 101);
    glutAddMenuEntry("Verde", 102);

    int cintaSplineSubMenu = glutCreateMenu(optionHandler);
    glutAddMenuEntry("Rojo", 103);
    glutAddMenuEntry("Verde", 104);
    glutAddMenuEntry("Azul", 105);
    glutAddMenuEntry("Aleatorio", 106);

    int graficarCurvaSubMenu = glutCreateMenu(optionHandler);
    glutAddMenuEntry("Activado", 11);
    glutAddMenuEntry("Desactivado", 13);

    int mainMenu = glutCreateMenu(optionHandler);
    glutAddMenuEntry("Habilitar/Deshabilitar Ingresar puntos de control", 1);
    glutAddMenuEntry("Mostrar/Ocultar polilinea", 2);
    glutAddMenuEntry("Mostrar/Ocultar curva 2D", 30);
    glutAddMenuEntry("Mostrar/Ocultar puntos de control", 3);
    glutAddMenuEntry("Mostrar Curva B-Spline/Bezier", 4);
    glutAddMenuEntry("Habilitar/Deshabilitar Mover Puntos de control", 5);
    glutAddMenuEntry("Mostrar/Ocultar Ejes coordenados 2D", 6);
    glutAddSubMenu("Cambiar color de la curva 2D", colorSubMenu);
    glutAddSubMenu("Cambiar el grosor de la linea", grosorSubMenu);
    glutAddSubMenu("Cambiar color de la cinta Poligonal", cintaPoligonalSubMenu);
    glutAddSubMenu("Cambiar color de la curva Spline/Bezier", cintaSplineSubMenu);
    glutAddSubMenu("Graficar curva en 3D", graficarCurvaSubMenu);

    glutAddMenuEntry("Cambiar el color del fondo", 9);
    glutAddMenuEntry("Salir de la aplicacion", 12);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void init() {
    // Establecer el color de fondo y crear el menú
    glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.0f);
}

void mensajeInicial() {
    cout << "Referencias:\n"
        << "   - Codigo base que utilizamos para empezar el programa, algoritmo de Bezier: https://github.com/kodiidok/glut-bezier-curves\n"
        << "   - Informacion adicional de algunas funciones en OpenGL: https://www.fing.edu.uy/inco/cursos/compgraf/Clases/2012/09-Mallas%20poligonales%20y%20curvas%5BopenGL%5D.pdf\n\n"
        << "Autores:\n"
        << "   - Said William Najarro Llacza - 22200031\n"
        << "   - José Roberto Sernaque Gutierrez - 22200063\n\n"
        << "Agradecimientos especiales:\n"
        << "   - ChatGPT: https://chatgpt.com/ para la lógica del mouse y movimiento de los puntos de control.\n"
        << "\n";
}

int main(int argc, char** argv) {
    mensajeInicial();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(50, 200);
    glutCreateWindow("Puntos de control y curvas en 2D y 3D");
    createMenu();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    init();
    glutMainLoop();
    return 0;
}