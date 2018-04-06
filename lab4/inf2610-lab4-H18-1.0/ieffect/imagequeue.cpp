#include <QImage>
#include <QDebug>
#include <QQueue>

#include "imagequeue.h"
#include "simpletracer.h"
#include "windows.h"
#include "stdio.h"

HANDLE effectSemaphore;
HANDLE loadSemaphore;
HANDLE queueMutex;

ImageQueue::ImageQueue(QObject *parent, int capacity) :
    QObject(parent),
    m_capacity(capacity)
{
    //applique pour Dequeue. Initialise a 0 car la file est initialement vide. Lorsque enqueue place un image dans
    //la liste temporaire, il permet alors a dequeue de recuperer une image de la queue.
    //Permet d'empecher un dequeue sur une file vide
    effectSemaphore = CreateSemaphore(
                NULL,
                0,
                capacity,
                NULL);
    // Utilise sur enqueue. Initialise au nombre d image presente.
    // Permet d empecher que enqueue aille chercher une image dans une liste vide.
    // Est decremente chaque fois qu une image est transferee
    loadSemaphore = CreateSemaphore(
                NULL,
                capacity,
                capacity,
                NULL);

    // Empeche la queue de subir un dequeue et un queue au meme moment et avoir des semaphorer erronne
    // ce qui menerait a l interblocage

    queueMutex = CreateMutex(
                NULL,
                TRUE,
                FALSE);
    // TODO
}

ImageQueue::~ImageQueue()
{
    // TODO
    CloseHandle(effectSemaphore);
    CloseHandle(loadSemaphore);
    CloseHandle(queueMutex);
}

void ImageQueue::enqueue(QImage *item)
{
    // On verifie que la liste d image a transferer n est pas nulle avec le loadSemaphore
    // et on attend d obtenir le mutex
    WaitForSingleObject(loadSemaphore, INFINITE);
    WaitForSingleObject(queueMutex, INFINITE);
    queue.enqueue(item);
    SimpleTracer::writeEvent(this, 0);
    ReleaseMutex(queueMutex);
    ReleaseSemaphore(effectSemaphore, 1, NULL);
    // TODO
    // tracer la taille de la file lorsqu'elle change
}

QImage *ImageQueue::dequeue()
{
    // TODO
    // tracer la taille de la file lorsqu'elle change
    // On attend que le effectSephamore soit non nul, ce qui
    // signifie que la queue contient des images a traiter
    WaitForSingleObject(effectSemaphore, INFINITE);
    WaitForSingleObject(queueMutex, INFINITE);
    QImage *image = queue.dequeue();
    SimpleTracer::writeEvent(this, 0);
    ReleaseMutex(queueMutex);
    ReleaseSemaphore(loadSemaphore, 1, NULL);
    return image;
}
