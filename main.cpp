#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <limits>
#include <clocale>

using namespace std;

// --- Estructura exacta del Nodo ---
struct Nodo {
    int prioridad;
    Nodo *siguiente;
    string tarea;
};

// --- Funciones auxiliares ---
int prioridadTotal(int urgencia, int importancia) {
    return urgencia * 10 + importancia;
}

// Transformación a minúsculas para estandarizar búsquedas
string aMinusculas(string str) {
    transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return tolower(c);
    });
    return str;
}

// --- Funciones de la Lista Ligada Ordenada ---
void insertarLista(Nodo *&lista, Nodo *nuevo_nodo) {
    if (nuevo_nodo == nullptr) return;

    Nodo *aux1 = lista;
    Nodo *aux2 = nullptr;

    // Recorre para ordenar de mayor a menor prioridad
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

void mostrarLista(Nodo *lista) {
    if (lista == nullptr) {
        cout << "\n[!] La lista de tareas pendientes está vacía.\n";
        return;
    }

    Nodo *actual = lista;
    int p = 0;
    cout << "\n=== TAREAS PENDIENTES (Ordenadas por Prioridad) ===\n";
    while (actual != nullptr) {
        cout << p + 1 << ". [ " << actual->tarea << " ] (Prioridad: " << actual->prioridad << ")\n";
        actual = actual->siguiente;
        p++;
    }
    cout << "===================================================\n";
}

void eliminarNodo(Nodo *&lista, string tarea) {
    if (lista == nullptr) {
        cout << "\n[!] La lista de tareas está vacía.\n";
        return;
    }

    string busqueda = aMinusculas(tarea);
    Nodo *aux_borrar = lista;
    Nodo *anterior = nullptr;

    while ((aux_borrar != nullptr) && (aux_borrar->tarea != busqueda)) {
        anterior = aux_borrar;
        aux_borrar = aux_borrar->siguiente;
    }

    if (aux_borrar == nullptr) {
        cout << "\n[!] La tarea '" << tarea << "' no fue encontrada.\n";
    } else if (anterior == nullptr) {
        lista = lista->siguiente;
        delete aux_borrar;
        cout << "\n[?] Tarea eliminada con éxito.\n";
    } else {
        anterior->siguiente = aux_borrar->siguiente;
        delete aux_borrar;
        cout << "\n[?] Tarea eliminada con éxito.\n";
    }
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

void mostrarCompletadas(Nodo *pila) {
    if (pila == nullptr) {
        cout << "\n[!] No hay tareas en el historial de completadas.\n";
        return;
    }

    Nodo *actual = pila;
    int p = 0;
    cout << "\n=== HISTORIAL DE TAREAS COMPLETADAS (Pila) ===\n";
    while (actual != nullptr) {
        cout << p + 1 << ". [ " << actual->tarea << " ] (Prioridad original: " << actual->prioridad << ")\n";
        actual = actual->siguiente;
        p++;
    }
    cout << "==============================================\n";
}

void eliminarCompletada(Nodo *&pila, string tarea) {
    if (pila == nullptr) {
        cout << "\n[!] No hay tareas completadas para eliminar.\n";
        return;
    }

    string busqueda = aMinusculas(tarea);
    Nodo *actual = pila;
    Nodo *anterior = nullptr;

    while (actual != nullptr && actual->tarea != busqueda) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == nullptr) {
        cout << "\n[!] No se encontró la tarea '" << tarea << "' en el historial de completadas.\n";
        return;
    }

    if (anterior == nullptr) {
        pila = actual->siguiente;
    } else {
        anterior->siguiente = actual->siguiente;
    }

    delete actual;
    cout << "\n[?] Tarea completada eliminada del historial permanentemente.\n";
}

void vaciarCompletadas(Nodo *&pila) {
    eliminarLista(pila);
}

// --- Operaciones de Movimiento entre Lista y Pila ---
void completarTarea(Nodo *&lista, Nodo *&completadas, string tarea) {
    if (lista == nullptr) {
        cout << "\n[!] La lista de pendientes está vacía.\n";
        return;
    }

    string busqueda = aMinusculas(tarea);
    Nodo *actual = lista;
    Nodo *anterior = nullptr;

    while (actual != nullptr && actual->tarea != busqueda) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == nullptr) {
        cout << "\n[!] No se encontró la tarea '" << tarea << "'.\n";
        return;
    }

    if (anterior == nullptr) {
        lista = actual->siguiente;
    } else {
        anterior->siguiente = actual->siguiente;
    }
    actual->siguiente = nullptr;

    agregarPila(completadas, actual);
    cout << "\n[?] Tarea completada correctamente.\n";
}

void completarTodas(Nodo *&lista, Nodo *&completadas) {
    if (lista == nullptr) {
        cout << "\n[!] No hay tareas pendientes para completar.\n";
        return;
    }

    int contador = 0;
    while (lista != nullptr) {
        Nodo *aux = lista;
        lista = lista->siguiente;
        aux->siguiente = nullptr;
        agregarPila(completadas, aux);
        contador++;
    }
    cout << "\n[?] Se completaron todas las tareas (" << contador << " movidas a la pila).\n";
}

// ===== FUNCIÓN PRINCIPAL =====
int main() {
    setlocale(LC_ALL, "spanish");

    Nodo *lista = nullptr;
    Nodo *completadas = nullptr;
    int opcion = 0;

    do {
        cout << "\n=========================================\n";
        cout << "            .: MENÚ DE TAREAS :.\n";
        cout << "=========================================\n";
        cout << " 1. Insertar tarea con prioridad\n";
        cout << " 2. Mostrar lista de tareas pendientes\n";
        cout << " 3. Completar una tarea\n";
        cout << " 4. Completar TODAS las tareas\n";
        cout << " 5. Eliminar una tarea pendiente\n";
        cout << " 6. Eliminar TODAS las tareas pendientes\n";
        cout << " 7. Restaurar última tarea completada (Undo)\n";
        cout << " 8. Mostrar historial de completadas\n";
        cout << " 9. Eliminar una tarea completada específica\n";
        cout << "10. Vaciar historial de completadas\n";
        cout << "11. Salir\n";
        cout << "-----------------------------------------\n";
        cout << "Digite una opción: ";

        if (!(cin >> opcion)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\n[!] Opción no válida. Intente de nuevo.\n";
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        string tarea_input;
        int urgencia = 1, importancia = 1;

        switch (opcion) {
            case 1:
                cout << "\nIngrese la tarea: ";
                getline(cin, tarea_input);

                cout << "\nIngrese el grado de urgencia:\n"
                     << "[3. Muy urgente]\n"
                     << "[2. Urgente]\n"
                     << "[1. Poco urgente]\n> ";
                cin >> urgencia;

                cout << "\nIngrese el grado de importancia:\n"
                     << "[3. Muy importante]\n"
                     << "[2. Importante]\n"
                     << "[1. Poco importante]\n> ";
                cin >> importancia;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                {
                    string tarea_original = tarea_input;
                    insertarLista(lista, urgencia, importancia, tarea_input);
                    cout << "\nTarea '" << tarea_original << "' registrada correctamente (Prioridad: " 
                         << prioridadTotal(urgencia, importancia) << ").\n";
                }
                break;

            case 2:
                mostrarLista(lista);
                break;

            case 3:
                cout << "\nIngrese la tarea que desea marcar como completada:\n> ";
                getline(cin, tarea_input);
                completarTarea(lista, completadas, tarea_input);
                break;

            case 4:
                completarTodas(lista, completadas);
                break;

            case 5:
                if (lista == nullptr) {
                    cout << "\n[!] La lista de tareas está vacía.\n";
                } else {
                    cout << "\nIngrese la tarea que desea eliminar:\n> ";
                    getline(cin, tarea_input);

                    cout << "\n¿Está seguro de que desea eliminar la tarea '" << tarea_input << "'? [S / N]\n> ";
                    char ans;
                    cin >> ans;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (tolower(ans) == 's') {
                        eliminarNodo(lista, tarea_input);
                    } else {
                        cout << "\n[!] Operación cancelada. La tarea no fue eliminada.\n";
                    }
                }
                break;

            case 6:
                if (lista == nullptr) {
                    cout << "\nLa lista de tareas está vacía.\n";
                } else {
                    cout << "\n¿Está seguro de eliminar todas las tareas? [S / N]\n> ";
                    char ans;
                    cin >> ans;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (tolower(ans) == 's') {
                        eliminarLista(lista);
                        cout << "\nLista eliminada por completo.\n";
                    }
                }
                break;

            case 7: {
                Nodo *restaurada = removerPila(completadas);
                if (restaurada != nullptr) {
                    insertarLista(lista, restaurada);
                    cout << "\nTarea restaurada con éxito a la lista de pendientes.\n";
                } else {
                    cout << "\nNo hay tareas completadas para restaurar.\n";
                }
                break;
            }

            case 8:
                mostrarCompletadas(completadas);
                break;

            case 9:
                cout << "\nIngrese la tarea completada que desea eliminar:\n> ";
                getline(cin, tarea_input);
                eliminarCompletada(completadas, tarea_input);
                break;

            case 10:
                if (completadas == nullptr) {
                    cout << "\nEl historial de completadas ya está vacío.\n";
                } else {
                    cout << "\n¿Está seguro de vaciar el historial de completadas? [S / N]\n> ";
                    char ans;
                    cin >> ans;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (tolower(ans) == 's') {
                        vaciarCompletadas(completadas);
                        cout << "\nHistorial de completadas vaciado completamente.\n";
                    }
                }
                break;

            case 11:
                cout << "\nSaliendo del programa...\n";
                eliminarLista(lista);
                vaciarCompletadas(completadas);
                break;

            default:
                cout << "\nOpción no válida. Intente de nuevo.\n";
        }

    } while (opcion != 11);

    return 0;
}
