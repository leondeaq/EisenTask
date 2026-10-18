#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <emscripten/emscripten.h>

using namespace std;

// --- Estructura del Nodo de la Lista Ligada ---
struct Nodo {
    string tarea;
    int urgencia;
    int importancia;
    int prioridad;
    Nodo* siguiente;

    Nodo(string t, int u, int i, int p) {
        tarea = t;
        urgencia = u;
        importancia = i;
        prioridad = p;
        siguiente = nullptr;
    }
};

// --- Puntero Global en la RAM del navegador ---
Nodo* listaGlobal = nullptr;

// --- Buffer global para enviar JSON a JavaScript ---
string bufferSalida;

// --- Funciones auxiliares internas ---
int calcularPrioridadTotal(int urgencia, int importancia) {
    return (urgencia * 10) + importancia;
}

string aMinusculas(string str) {
    transform(str.begin(), str.end(), str.begin(), [](unsigned char c){
        return tolower(c);
    });
    return str;
}

// =================================================================
// FUNCIONES EXPORTADAS A WEBASSEMBLY (JAVASCRIPT)
// =================================================================
extern "C" {

    // 1. Insertar tarea en orden descendente de prioridad
    EMSCRIPTEN_KEEPALIVE
    void insertarTarea(const char* c_tarea, int urgencia, int importancia) {
        string tarea = aMinusculas(string(c_tarea));
        int prioridad = calcularPrioridadTotal(urgencia, importancia);

        Nodo* nuevo_nodo = new Nodo(tarea, urgencia, importancia, prioridad);

        if (listaGlobal == nullptr || nuevo_nodo->prioridad > listaGlobal->prioridad) {
            nuevo_nodo->siguiente = listaGlobal;
            listaGlobal = nuevo_nodo;
            return;
        }

        Nodo* aux1 = listaGlobal;
        Nodo* aux2 = nullptr;

        while (aux1 != nullptr && aux1->prioridad >= nuevo_nodo->prioridad) {
            aux2 = aux1;
            aux1 = aux1->siguiente;
        }

        aux2->siguiente = nuevo_nodo;
        nuevo_nodo->siguiente = aux1;
    }

    // 2. Obtener lista completa en formato JSON para la interfaz web
    EMSCRIPTEN_KEEPALIVE
    const char* obtenerListaJSON() {
        bufferSalida = "[";
        Nodo* actual = listaGlobal;

        while (actual != nullptr) {
            bufferSalida += "{\"tarea\":\"" + actual->tarea + "\",";
            bufferSalida += "\"urgencia\":" + to_string(actual->urgencia) + ",";
            bufferSalida += "\"importancia\":" + to_string(actual->importancia) + ",";
            bufferSalida += "\"prioridad\":" + to_string(actual->prioridad) + "}";

            if (actual->siguiente != nullptr) {
                bufferSalida += ",";
            }
            actual = actual->siguiente;
        }

        bufferSalida += "]";
        return bufferSalida.c_str();
    }

    // 3. Eliminar una tarea especÍfica por su nombre
    EMSCRIPTEN_KEEPALIVE
    int eliminarTarea(const char* c_tarea) {
        if (listaGlobal == nullptr) return 0;

        string tarea = aMinusculas(string(c_tarea));
        Nodo* aux_borrar = listaGlobal;
        Nodo* anterior = nullptr;

        while (aux_borrar != nullptr && aux_borrar->tarea != tarea) {
            anterior = aux_borrar;
            aux_borrar = aux_borrar->siguiente;
        }

        if (aux_borrar == nullptr) {
            return 0; // No encontrada
        }

        if (anterior == nullptr) {
            listaGlobal = listaGlobal->siguiente;
        } else {
            anterior->siguiente = aux_borrar->siguiente;
        }

        delete aux_borrar; // Liberar memoria dinámica
        return 1; // Éxito
    }

    // 4. Eliminar la tarea más prioritaria (el frente)
    EMSCRIPTEN_KEEPALIVE
    int eliminarFrente() {
        if (listaGlobal == nullptr) return 0;

        Nodo* temp = listaGlobal;
        listaGlobal = listaGlobal->siguiente;
        delete temp;
        return 1;
    }

    // 5. Vaciar toda la lista y liberar memoria dinámica
    EMSCRIPTEN_KEEPALIVE
    void vaciarLista() {
        while (listaGlobal != nullptr) {
            Nodo* aux = listaGlobal;
            listaGlobal = listaGlobal->siguiente;
            delete aux;
        }
    }
}
