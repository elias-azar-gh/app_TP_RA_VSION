//
//  ArUco-OpenGL.cpp
//
//  Created by Jean-Marie Normand on 28/02/13.
//  Copyright (c) 2013 Centrale Nantes. All rights reserved.
//


#include "ArUco-OpenGL.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2\calib3d.hpp>
#include <GL/glu.h> // Inclure GLU pour utiliser gluSphere
#include <cmath>

// Constructor
ArUco::ArUco(string intrinFileName, float markerSize) {
    // Initializing attributes
    m_IntrinsicFile = intrinFileName;
    m_MarkerSize = markerSize;
    // read camera parameters if passed
    m_CameraParams.readFromXMLFile(intrinFileName);

}

// Destructor
ArUco::~ArUco() {}

void ArUco::resizeCameraParams(cv::Size newSize) {
    m_CameraParams.resize(newSize);
}

// Detect marker and draw things
void ArUco::doWork(Mat inputImg) {
    m_InputImage = inputImg;
    m_GlWindowSize = m_InputImage.size();
    m_CameraParams.resize(m_InputImage.size());
    resize(m_GlWindowSize.width, m_GlWindowSize.height);
}

//Fonction pour gérer la rotation des sphères
void ArUco::updateRotation() {
    rotationAngle += 1.0f; // Incrément de 1 degré par frame (ajustez selon vos besoins)
    if (rotationAngle >= 360.0f) {
        rotationAngle -= 360.0f; // Évitez de dépasser 360 degrés
    }
}

// Fonction pour dessiner une sphère
void ArUco::drawSphere(float radius, int slices, int stacks) {

    GLUquadric* quadric = gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluSphere(quadric, radius, slices, stacks);
    gluDeleteQuadric(quadric);

}

void ArUco::drawSphereWithRotation(float radius, int slices, int stacks) {
    glPushMatrix(); // Sauvegarder la matrice courante

    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f); // Faire tourner la sphère autour de l'axe Y
    drawSphere(radius, slices, stacks);         // Dessiner la sphère

    glPopMatrix(); // Restaurer la matrice
}

double calculateDistance(double* pos1, double* pos2) {
    return sqrt(pow(pos2[0] - pos1[0], 2) + pow(pos2[1] - pos1[1], 2) + pow(pos2[2] - pos1[2], 2));
}

// Dessiner la scène avec des sphères
void ArUco::drawScene() {
    // Si aucune image n'est disponible, on ne fait rien
    if (m_ResizedImage.rows == 0)
        return;

    // Réinitialisation des matrices OpenGL
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Vue orthographique pour l'image OpenCV
    glOrtho(0, m_GlWindowSize.width, 0, m_GlWindowSize.height, -1.0, 1.0);
    glViewport(0, 0, m_GlWindowSize.width, m_GlWindowSize.height);

    // Désactiver les textures
    glDisable(GL_TEXTURE_2D);

    // Inversion de l'axe Y pour aligner OpenCV et OpenGL
    glPixelZoom(1, -1);

    // Afficher l'image de la webcam comme fond
    glRasterPos3f(0, m_GlWindowSize.height, -1.0f);
    glDrawPixels(m_GlWindowSize.width, m_GlWindowSize.height, GL_RGB, GL_UNSIGNED_BYTE, m_ResizedImage.ptr(0));

    // Activer le depth test pour les objets 3D
    glEnable(GL_DEPTH_TEST);

    // Configurer la projection pour la caméra
    glMatrixMode(GL_PROJECTION);
    double proj_matrix[16];
    m_CameraParams.glGetProjectionMatrix(m_ResizedImage.size(), m_GlWindowSize, proj_matrix, 0.01, 100);
    glLoadIdentity();
    glLoadMatrixd(proj_matrix);

    // On desactive le depth test
    glDisable(GL_DEPTH_TEST);

    // Dessiner les objets pour chaque marqueur
    double modelview_matrix[16];
    double position_marker1[3]; // Position du premier marqueur (fixe)
    double position_marker2[3]; // Position du deuxième marqueur (orbite)

    for (unsigned int m = 0; m < m_Markers.size(); m++) {
        m_Markers[m].glGetModelViewMatrix(modelview_matrix);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLoadMatrixd(modelview_matrix);

        // Se déplacer sur Z pour dessiner sur le marqueur
        glTranslatef(0, 0, m_MarkerSize / 2.);

        // Sauvegarder la matrice courante
        glPushMatrix();

        // Définir la couleur de la sphère
        glColor3f(0.2f * m, 0.5f, 0.8f); // Couleur variable selon l'indice du marqueur

        // Dessiner la sphère du premier marqueur (fixe)
        if (m == 0) {
            // Récupérer la position du premier marqueur (fixe)
            position_marker1[0] = modelview_matrix[12];
            position_marker1[1] = modelview_matrix[13];
            position_marker1[2] = modelview_matrix[14];

            float radius1 = m_MarkerSize;
            drawSphereWithRotation(radius1, 20, 20); // Sphère fixe
        }

        // Dessiner la sphère du deuxième marqueur (orbite)
        if (m == 1) {
            // Récupérer la position du deuxième marqueur
            position_marker2[0] = modelview_matrix[12];
            position_marker2[1] = modelview_matrix[13];
            position_marker2[2] = modelview_matrix[14];

            // Calculer la distance entre les deux marqueurs
            double distance = calculateDistance(position_marker1, position_marker2);

            // Animer la sphère autour du premier marqueur
            static float angle = 0.0f; // Variable statique pour l'animation

            // Déplacer la sphère sur une orbite circulaire
            glPushMatrix();

            // Calculer la nouvelle position de la sphère
            glTranslatef(position_marker1[0], position_marker1[1], position_marker1[2]); // Se déplacer à la position du premier marqueur
            glRotatef(angle, 0.0f, 0.0f, 1.0f); // Effectuer une rotation autour de l'axe Z

            // Se déplacer selon le rayon de l'orbite (distance calculée)
            glTranslatef(distance, 0.0f, 0.0f); // Rayon de l'orbite sur l'axe X

            // Dessiner la sphère qui orbite
            float radius2 = m_MarkerSize * 0.5f;
            drawSphereWithRotation(radius2, 20, 20); // Sphère en orbite

            glPopMatrix();

            // Incrémenter l'angle pour la prochaine position
            angle += 1.0f;
            if (angle >= 360.0f) {
                angle = 0.0f; // Remettre l'angle à zéro après un tour complet
            }
        }

        // Dessiner une sphère avec un rayon différent pour chaque marqueur
        float radius = m_MarkerSize * 0.3f + 0.02f * m; // Rayon basé sur l'indice du marqueur
        drawSphereWithRotation(radius, 20, 20); // Sphère lissée en rotation

        // Restaurer la matrice précédente
        glPopMatrix();
    }

    // Désactiver le depth test
    glDisable(GL_DEPTH_TEST);
}




// Idle function
void ArUco::idle(Mat newImage) {

    // Met à jour l'angle de rotation
    updateRotation();

    // Getting new image
    m_InputImage = newImage.clone();

    // Undistort image based on distorsion parameters
    m_UndInputImage.create(m_InputImage.size(), CV_8UC3);

    //transform color that by default is BGR to RGB because windows systems do not allow reading BGR images with opengl properly
    cv::cvtColor(m_InputImage, m_InputImage, cv::COLOR_BGR2RGB);

    //remove distorion in image ==> does not work very well (the YML file is not that of my camera)
    //cv::undistort(m_InputImage,m_UndInputImage, m_CameraParams.CameraMatrix, m_CameraParams.Distorsion);
    m_UndInputImage = m_InputImage.clone();

    //resize the image to the size of the GL window
    cv::resize(m_UndInputImage, m_ResizedImage, m_GlWindowSize);

    //detect markers
    m_PPDetector.detect(m_ResizedImage, m_Markers, m_CameraParams, m_MarkerSize, false);

}

// Resize function
void ArUco::resize(GLsizei iWidth, GLsizei iHeight) {
    m_GlWindowSize = Size(iWidth, iHeight);

    //not all sizes are allowed. OpenCv images have padding at the end of each line in these that are not aligned to 4 bytes
    if (iWidth * 3 % 4 != 0) {
        iWidth += iWidth * 3 % 4;//resize to avoid padding
        resize(iWidth, m_GlWindowSize.height);
    }
    else {
        //resize the image to the size of the GL window
        if (m_UndInputImage.rows != 0)
            cv::resize(m_UndInputImage, m_ResizedImage, m_GlWindowSize);
    }

}

// Test using ArUco to display a 3D cube in OpenCV
void ArUco::draw3DCube(cv::Mat img, int markerInd) {
    if (m_Markers.size() > markerInd) {
        aruco::CvDrawingUtils::draw3dCube(img, m_Markers[markerInd], m_CameraParams);
    }
}

void ArUco::draw3DAxis(cv::Mat img, int markerInd) {
    if (m_Markers.size() > markerInd) {
        aruco::CvDrawingUtils::draw3dAxis(img, m_Markers[markerInd], m_CameraParams);
    }

}
