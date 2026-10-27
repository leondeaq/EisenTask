#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <emscripten/emscripten.h>

using namespace std;

// --- Estructura exacta del Nodo ---
struct Nodo {
    int prioridad;
    Nodo *siguiente;
    string tarea;
};

// --- Punteros Globales en la RAM del navegador ---
Nodo* listaGlobal = nullptr;          // Lista de tareas pendientes
Nodo* pilaCompletadasGlobal = nullptr; // Pila de tareas completadas

// --- Buffers globales para enviar JSON a JavaScript ---
string bufferPendientesJSON;
string bufferCompletadasJSON;

// --- Funciones auxiliares internas ---
int prioridadTotal(int urgencia, int importancia) {
    return (urgencia * 10) + importancia;
}

string aMinusculas(string str) {
    transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return tolower(c);
    });
    return str;
}

string escaparJSON(string str) {
    string res = "";
    for (char c : str) {
        if (c == '"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else res += c;
    }
    return res;
}

// --- Funciones de la Lista Ligada Ordenada ---
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

    if (aux_borrar == nullptr) return;

    if (anterior == nullptr) {
        lista = lista->siguiente;
    } else {
        anterior->siguiente = aux_borrar->siguiente;
    }

    delete aux_borrar;
}

void eliminarLista(Nodo *&lista) {
    while (lista != nullptr) {
        Nodo *aux = lista;
        lista = aux->siguiente;
        delete aux;
    }
}

// --- Funciones de la Pila de Completadas ---
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

// --- Operaciones entre Lista y Pila ---
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
    while (lista != nullptr) {
        Nodo *aux = lista;
        lista = lista->siguiente;
        aux->siguiente = nullptr;
        agregarPila(completadas, aux);
    }
}

// =================================================================
// FUNCIONES EXPORTADAS A WEBASSEMBLY (JAVASCRIPT)
// =================================================================
extern "C" {

    // 1. Insertar tarea pendiente con prioridad
    EMSCRIPTEN_KEEPALIVE
    void insertarTarea(const char* c_tarea, int urgencia, int importancia) {
        insertarLista(listaGlobal, urgencia, importancia, string(c_tarea));
    }

    // 2. Obtener la lista de pendientes en JSON
    EMSCRIPTEN_KEEPALIVE
    const char* obtenerListaJSON() {
        bufferPendientesJSON = "[";
        Nodo* actual = listaGlobal;

        while (actual != nullptr) {
            bufferPendientesJSON += "{\"tarea\":\"" + escaparJSON(actual->tarea) + "\",";
            bufferPendientesJSON += "\"prioridad\":" + to_string(actual->prioridad) + "}";

            if (actual->siguiente != nullptr) {
                bufferPendientesJSON += ",";
            }
            actual = actual->siguiente;
        }

        bufferPendientesJSON += "]";
        return bufferPendientesJSON.c_str();
    }

    // 3. Obtener el historial de completadas (Pila) en JSON
    EMSCRIPTEN_KEEPALIVE
    const char* obtenerCompletadasJSON() {
        bufferCompletadasJSON = "[";
        Nodo* actual = pilaCompletadasGlobal;

        while (actual != nullptr) {
            bufferCompletadasJSON += "{\"tarea\":\"" + escaparJSON(actual->tarea) + "\",";
            bufferCompletadasJSON += "\"prioridad\":" + to_string(actual->prioridad) + "}";

            if (actual->siguiente != nullptr) {
                bufferCompletadasJSON += ",";
            }
            actual = actual->siguiente;
        }

        bufferCompletadasJSON += "]";
        return bufferCompletadasJSON.c_str();
    }

    // 4. Completar una tarea especifica (mover a la Pila)
    EMSCRIPTEN_KEEPALIVE
    int completarTareaWasm(const char* c_tarea) {
        completarTarea(listaGlobal, pilaCompletadasGlobal, string(c_tarea));
        return 1;
    }

    // 5. Completar TODAS las tareas (mover todas a la Pila)
    EMSCRIPTEN_KEEPALIVE
    int completarTodasWasm() {
        completarTodas(listaGlobal, pilaCompletadasGlobal);
        return 1;
    }

    // 6. Eliminar una tarea pendiente (delete directo en C++)
    EMSCRIPTEN_KEEPALIVE
    int eliminarTareaWasm(const char* c_tarea) {
        eliminarNodo(listaGlobal, string(c_tarea));
        return 1;
    }

    // 7. Eliminar TODAS las tareas pendientes
    EMSCRIPTEN_KEEPALIVE
    int eliminarTodasWasm() {
        eliminarLista(listaGlobal);
        return 1;
    }

    // 8. Restaurar la ultima tarea completada (Undo de la Pila a la Lista)
    EMSCRIPTEN_KEEPALIVE
    int restaurarUltimaWasm() {
        Nodo* restaurada = removerPila(pilaCompletadasGlobal);
        if (restaurada != nullptr) {
            insertarLista(listaGlobal, restaurada);
            return 1;
        }
        return 0;
    }

    // 9. Eliminar una tarea especifica del historial de completadas
    EMSCRIPTEN_KEEPALIVE
    int eliminarCompletadaWasm(const char* c_tarea) {
        eliminarCompletada(pilaCompletadasGlobal, string(c_tarea));
        return 1;
    }

    // 10. Vaciar todo el historial de completadas
    EMSCRIPTEN_KEEPALIVE
    void vaciarCompletadasWasm() {
        vaciarCompletadas(pilaCompletadasGlobal);
    }
}