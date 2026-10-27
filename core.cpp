#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <emscripten/emscripten.h>

using namespace std;

// --- Estructura exacta de tu Nodo original ---
struct Nodo {
    int prioridad;
    Nodo *siguiente;
    string tarea;
};

// --- Punteros globales de tu programa original ---
Nodo *lista = nullptr;
Nodo *completadas = nullptr;

// --- Buffers globales para enviar JSON a JavaScript ---
string bufferListaJSON;
string bufferCompletadasJSON;

// --- Tus funciones auxiliares originales ---
int prioridadTotal(int urgencia, int importancia) {
    return urgencia * 10 + importancia;
}

string aMinusculas(string str) {
    transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return tolower(c);
    });
    return str;
}

// --- Tus funciones originales de la Lista Ligada Ordenada ---
void insertarLista(Nodo *&lista, Nodo *nuevo_nodo) {
    if (nuevo_nodo == nullptr) return;

    Nodo *aux1 = lista;
    Nodo *aux2 = nullptr;

    while ((aux1 != nullptr) && (aux1->prioridad >= nuevo_nodo->prioridad)) {
        aux2 = aux1;
        aux1 = aux1->siguiente;
    }

    if (lista == aux1) {
        lista = nuevo_nodo;
    } else {
        aux2->siguiente = nuevo_nodo;
    }

    nuevo_nodo->siguiente = aux1;
}

void insertarLista(Nodo *&lista, int urgencia, int importancia, string tarea) {
    Nodo *nuevo_nodo = new Nodo();
    nuevo_nodo->tarea = aMinusculas(tarea);
    nuevo_nodo->prioridad = prioridadTotal(urgencia, importancia);
    nuevo_nodo->siguiente = nullptr;

    insertarLista(lista, nuevo_nodo);
}

void eliminarNodo(Nodo *&lista, string tarea) {
    if (lista == nullptr) return;

    string busqueda = aMinusculas(tarea);
    Nodo *aux_borrar = lista;
    Nodo *anterior = nullptr;

    while ((aux_borrar != nullptr) && (aux_borrar->tarea != busqueda)) {
        anterior = aux_borrar;
        aux_borrar = aux_borrar->siguiente;
    }

    if (aux_borrar == nullptr) {
        return;
    } else if (anterior == nullptr) {
        lista = lista->siguiente;
        delete aux_borrar;
    } else {
        anterior->siguiente = aux_borrar->siguiente;
        delete aux_borrar;
    }
}

void eliminarLista(Nodo *&lista) {
    while (lista != nullptr) {
        Nodo *aux = lista;
        lista = aux->siguiente;
        delete aux;
    }
}

// --- Tus funciones originales de la Pila de Completadas ---
void agregarPila(Nodo *&pila, Nodo *&tarea) {
    if (tarea == nullptr) return;
    tarea->siguiente = pila;
    pila = tarea;
    tarea = nullptr;
}

Nodo *removerPila(Nodo *&pila) {
    if (pila == nullptr) return nullptr;
    Nodo *aux = pila;
    pila = pila->siguiente;
    aux->siguiente = nullptr;
    return aux;
}

void eliminarCompletada(Nodo *&pila, string tarea) {
    if (pila == nullptr) return;

    string busqueda = aMinusculas(tarea);
    Nodo *actual = pila;
    Nodo *anterior = nullptr;

    while (actual != nullptr && actual->tarea != busqueda) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == nullptr) return;

    if (anterior == nullptr) {
        pila = actual->siguiente;
    } else {
        anterior->siguiente = actual->siguiente;
    }

    delete actual;
}

void vaciarCompletadas(Nodo *&pila) {
    eliminarLista(pila);
}

// --- Tus operaciones originales de Movimiento entre Lista y Pila ---
void completarTarea(Nodo *&lista, Nodo *&completadas, string tarea) {
    if (lista == nullptr) return;

    string busqueda = aMinusculas(tarea);
    Nodo *actual = lista;
    Nodo *anterior = nullptr;

    while (actual != nullptr && actual->tarea != busqueda) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == nullptr) return;

    if (anterior == nullptr) {
        lista = actual->siguiente;
    } else {
        anterior->siguiente = actual->siguiente;
    }
    actual->siguiente = nullptr;

    agregarPila(completadas, actual);
}

void completarTodas(Nodo *&lista, Nodo *&completadas) {
    if (lista == nullptr) return;

    while (lista != nullptr) {
        Nodo *aux = lista;
        lista = lista->siguiente;
        aux->siguiente = nullptr;
        agregarPila(completadas, aux);
    }
}

// =================================================================
// PUENTE DE COMUNICACIÓN CON WEBASSEMBLY (JAVASCRIPT)
// =================================================================
extern "C" {

    EMSCRIPTEN_KEEPALIVE
    void insertarTarea(const char* c_tarea, int urgencia, int importancia) {
        insertarLista(lista, urgencia, importancia, string(c_tarea));
    }

    EMSCRIPTEN_KEEPALIVE
    const char* obtenerListaJSON() {
        bufferListaJSON = "[";
        Nodo* actual = lista;

        while (actual != nullptr) {
            bufferListaJSON += "{\"tarea\":\"" + actual->tarea + "\",";
            bufferListaJSON += "\"prioridad\":" + to_string(actual->prioridad) + "}";

            if (actual->siguiente != nullptr) {
                bufferListaJSON += ",";
            }
            actual = actual->siguiente;
        }

        bufferListaJSON += "]";
        return bufferListaJSON.c_str();
    }

    EMSCRIPTEN_KEEPALIVE
    const char* obtenerCompletadasJSON() {
        bufferCompletadasJSON = "[";
        Nodo* actual = completadas;

        while (actual != nullptr) {
            bufferCompletadasJSON += "{\"tarea\":\"" + actual->tarea + "\",";
            bufferCompletadasJSON += "\"prioridad\":" + to_string(actual->prioridad) + "}";

            if (actual->siguiente != nullptr) {
                bufferCompletadasJSON += ",";
            }
            actual = actual->siguiente;
        }

        bufferCompletadasJSON += "]";
        return bufferCompletadasJSON.c_str();
    }

    EMSCRIPTEN_KEEPALIVE
    int completarTareaWasm(const char* c_tarea) {
        completarTarea(lista, completadas, string(c_tarea));
        return 1;
    }

    EMSCRIPTEN_KEEPALIVE
    int completarTodasWasm() {
        completarTodas(lista, completadas);
        return 1;
    }

    EMSCRIPTEN_KEEPALIVE
    int eliminarTareaWasm(const char* c_tarea) {
        eliminarNodo(lista, string(c_tarea));
        return 1;
    }

    EMSCRIPTEN_KEEPALIVE
    int eliminarTodasWasm() {
        eliminarLista(lista);
        return 1;
    }

    EMSCRIPTEN_KEEPALIVE
    int restaurarUltimaWasm() {
        Nodo *restaurada = removerPila(completadas);
        if (restaurada != nullptr) {
            insertarLista(lista, restaurada);
            return 1;
        }
        return 0;
    }

    EMSCRIPTEN_KEEPALIVE
    int eliminarCompletadaWasm(const char* c_tarea) {
        eliminarCompletada(completadas, string(c_tarea));
        return 1;
    }

    EMSCRIPTEN_KEEPALIVE
    void vaciarCompletadasWasm() {
        vaciarCompletadas(completadas);
    }
}

int main() {
    return 0;
}
