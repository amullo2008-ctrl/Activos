// Sistema de Gestion de Activos TI - TechSolutions S.A.
// Modulo: Grupo 4 - Activos TI
// Programacion 1 
// Integrantes: Angel Mullo, Juyani Santillan


#include <stdio.h>
#include <stdlib.h>

#define MAX_NOMBRE      80
#define MAX_TIPO        40
#define MAX_DEPTO       50
#define MAX_RESPONSABLE 60
#define MAX_ESTADO      15
#define ARCHIVO_DATOS   "activos.txt"

// MODULO 1: Estructuras de datos

/* Tipos de activo disponibles */
#define TIPO_PC        "PC/Laptop"
#define TIPO_SERVIDOR  "Servidor"
#define TIPO_RED       "Red/Switch"
#define TIPO_IMPRESORA "Impresora"
#define TIPO_OTRO      "Otro"

/* Estados posibles de un activo */
#define ESTADO_ACTIVO  "Activo"
#define ESTADO_BAJA    "De Baja"
#define ESTADO_MANT    "En Mantenimiento"

/* Representa un activo tecnologico de TechSolutions S.A. */
typedef struct {
    int  codigo;                    // identificador unico del activo
    char nombre[MAX_NOMBRE];        // nombre descriptivo del activo
    char tipo[MAX_TIPO];            // tipo de equipo
    char departamento[MAX_DEPTO];   // departamento donde se usa
    char responsable[MAX_RESPONSABLE]; // persona a cargo
    char estado[MAX_ESTADO];        // Activo | En Mantenimiento | De Baja
} Activo;

/* Nodo de la lista doblemente enlazada */
typedef struct Nodo {
    Activo dato;
    struct Nodo *anterior;
    struct Nodo *siguiente;
} Nodo;

/* Lista doblemente enlazada con cabeza, cola y contador */
typedef struct {
    Nodo *cabeza;
    Nodo *cola;
    int   cantidad;
} Lista;

// MODULO 2: Utilidades y validacion de entradas


// Quita el '\n' que fgets deja al final del string
void quitarSalto(char *s) {
    int i = 0;
    while (s[i] != '\0') i++;
    if (i > 0 && s[i-1] == '\n') s[i-1] = '\0';
}

// Copia manual de strings sin usar strcpy
void copiarStr(char *dst, const char *src) {
    int i = 0;
    while (src[i] != '\0') { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

// Compara dos strings ignorando mayusculas/minusculas
int strIguales(const char *a, const char *b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        char ca = a[i], cb = b[i];
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return 0;
        i++;
    }
    return (a[i] == '\0' && b[i] == '\0');
}

// Lee un entero valido >= minimo, rechaza letras y simbolos
int leerEntero(const char *msg, int minimo) {
    char buf[20];
    int valor, valido;
    do {
        valido = 1;
        printf("%s", msg);
        fgets(buf, sizeof(buf), stdin);
        quitarSalto(buf);
        int i = 0;
        if (buf[0] == '\0') { valido = 0; }
        while (buf[i] != '\0') {
            if (buf[i] < '0' || buf[i] > '9') { valido = 0; break; }
            i++;
        }
        if (valido) {
            valor = 0; i = 0;
            while (buf[i] != '\0') { valor = valor*10 + (buf[i]-'0'); i++; }
            if (valor < minimo) { printf("  [!] Debe ser >= %d.\n", minimo); valido = 0; }
        } else {
            printf("  [!] Entrada invalida. Solo numeros enteros.\n");
        }
    } while (!valido);
    return valor;
}

// Lee una opcion de un solo digito dentro del rango [mn, mx]
int leerOpcion(int mn, int mx) {
    char buf[10];
    int valor, valido;
    do {
        valido = 1;
        fgets(buf, sizeof(buf), stdin);
        quitarSalto(buf);
        if (buf[0] == '\0' || buf[1] != '\0') { valido = 0; }
        else {
            valor = buf[0] - '0';
            if (valor < mn || valor > mx) valido = 0;
        }
        if (!valido) printf("  [!] Opcion invalida. Ingrese entre %d y %d: ", mn, mx);
    } while (!valido);
    return valor;
}

// Lee texto obligatorio (no acepta campos vacios)
void leerTexto(const char *msg, char *buf, int tam) {
    int ok = 0;
    while (!ok) {
        printf("%s", msg);
        fgets(buf, tam, stdin);
        quitarSalto(buf);
        if (buf[0] != '\0') ok = 1;
        else printf("  [!] Campo obligatorio. No puede estar vacio.\n");
    }
}

// Reemplaza '|' por '_' para no romper el formato del archivo
void sanearCampo(char *s) {
    int i = 0;
    while (s[i] != '\0') {
        if (s[i] == '|') s[i] = '_';
        i++;
    }
}

// MODULO 3: Lista doblemente enlazada

// Inicializa la lista con punteros nulos (obligatorio antes de usar)
void listaInicializar(Lista *l) {
    l->cabeza = NULL;
    l->cola = NULL;
    l->cantidad = 0;
}

// Crea un nodo nuevo con malloc y lo agrega al final de la lista
Nodo* listaInsertarFinal(Lista *l, Activo a) {
    Nodo *nuevo = (Nodo*) malloc(sizeof(Nodo));
    if (nuevo == NULL) { printf("  [!] Error: sin memoria.\n"); return NULL; }

    nuevo->dato = a;
    nuevo->siguiente = NULL;
    nuevo->anterior = l->cola; // enlazar con el nodo anterior

    if (l->cola != NULL) l->cola->siguiente = nuevo;
    else l->cabeza = nuevo; // lista vacia: nuevo nodo es tambien cabeza

    l->cola = nuevo;
    l->cantidad++;
    return nuevo;
}

// Inserta un nodo de forma ordenada (ascendente por codigo)
Nodo* listaInsertarOrdenado(Lista *l, Activo a) {
    Nodo *nuevo = (Nodo*) malloc(sizeof(Nodo));
    if (nuevo == NULL) { printf("  [!] Error: sin memoria.\n"); return NULL; }
    nuevo->dato = a;
    nuevo->anterior = NULL;
    nuevo->siguiente = NULL;

    if (l->cabeza == NULL) {
        // lista vacia: el nodo es unico
        l->cabeza = l->cola = nuevo;
        l->cantidad++;
        return nuevo;
    }

    // Buscar la posicion correcta comparando codigos
    Nodo *actual = l->cabeza;
    while (actual != NULL && actual->dato.codigo < a.codigo)
        actual = actual->siguiente;

    if (actual == NULL) {
        // Insertar al final (codigo mayor que todos)
        nuevo->anterior = l->cola;
        l->cola->siguiente = nuevo;
        l->cola = nuevo;
    } else if (actual->anterior == NULL) {
        // Insertar al inicio (codigo menor que todos)
        nuevo->siguiente = l->cabeza;
        l->cabeza->anterior = nuevo;
        l->cabeza = nuevo;
    } else {
        // Insertar en medio: reconectar enlaces dobles
        nuevo->anterior = actual->anterior;
        nuevo->siguiente = actual;
        actual->anterior->siguiente = nuevo;
        actual->anterior = nuevo;
    }
    l->cantidad++;
    return nuevo;
}

// Busqueda lineal por codigo (recorre hacia adelante)
Nodo* listaBuscarPorCodigo(Lista *l, int codigo) {
    Nodo *actual = l->cabeza;
    while (actual != NULL) {
        if (actual->dato.codigo == codigo) return actual;
        actual = actual->siguiente;
    }
    return NULL;
}

// Elimina un nodo de la lista y libera su memoria (contempla 4 casos)
int listaEliminarNodo(Lista *l, Nodo *n) {
    if (n == NULL) return 0;
    if (n->anterior != NULL) n->anterior->siguiente = n->siguiente;
    else l->cabeza = n->siguiente; // n era la cabeza

    if (n->siguiente != NULL) n->siguiente->anterior = n->anterior;
    else l->cola = n->anterior;   // n era la cola

    free(n); // liberar memoria del nodo eliminado
    l->cantidad--;
    return 1;
}

// Ordena la lista por codigo usando burbuja sobre los datos (no los punteros)
void listaOrdenarPorCodigo(Lista *l) {
    if (l->cantidad < 2) return;
    int intercambiado;
    Nodo *actual;
    do {
        intercambiado = 0;
        actual = l->cabeza;
        while (actual != NULL && actual->siguiente != NULL) {
            if (actual->dato.codigo > actual->siguiente->dato.codigo) {
                // Intercambiar los datos (no los nodos) para mantener enlaces intactos
                Activo tmp = actual->dato;
                actual->dato = actual->siguiente->dato;
                actual->siguiente->dato = tmp;
                intercambiado = 1;
            }
            actual = actual->siguiente;
        }
    } while (intercambiado);
}

// Libera toda la memoria de la lista al cerrar el programa
void listaLiberar(Lista *l) {
    Nodo *actual = l->cabeza;
    while (actual != NULL) {
        Nodo *sig = actual->siguiente;
        free(actual);
        actual = sig;
    }
    listaInicializar(l);
}

// MODULO 4: Persistencia en archivo de texto
// Formato: codigo|nombre|tipo|departamento|responsable|estado

// Guarda todos los activos de la lista en el archivo, uno por linea
void archivoGuardar(Lista *l) {
    FILE *f = fopen(ARCHIVO_DATOS, "w");
    if (f == NULL) { printf("  [!] No se pudo guardar en archivo.\n"); return; }

    Nodo *actual = l->cabeza;
    while (actual != NULL) {
        Activo *a = &actual->dato;
        fprintf(f, "%d|%s|%s|%s|%s|%s\n",
                a->codigo, a->nombre, a->tipo,
                a->departamento, a->responsable, a->estado);
        actual = actual->siguiente;
    }
    fclose(f);
}

// Extrae un campo delimitado por '|' o '\n' desde la posicion *pos
static void leerCampo(const char *linea, int *pos, char *dest, int maxTam) {
    int j = 0;
    while (linea[*pos] != '\0' && linea[*pos] != '|' && linea[*pos] != '\n') {
        if (j < maxTam - 1) dest[j++] = linea[*pos];
        (*pos)++;
    }
    dest[j] = '\0';
    if (linea[*pos] == '|' || linea[*pos] == '\n') (*pos)++;
}

// Carga activos desde el archivo al iniciar el programa
void archivoCargar(Lista *l) {
    FILE *f = fopen(ARCHIVO_DATOS, "r");
    if (f == NULL) {
        printf("  [i] No se encontro '%s'. Se inicia con inventario vacio.\n", ARCHIVO_DATOS);
        return;
    }

    char linea[400];
    int total = 0;
    while (fgets(linea, sizeof(linea), f) != NULL) {
        int pos = 0;
        char campoCod[20];
        Activo a;

        leerCampo(linea, &pos, campoCod, sizeof(campoCod));
        if (campoCod[0] == '\0') continue;

        // Convertir codigo de string a entero manualmente
        int cod = 0, i = 0;
        while (campoCod[i] != '\0') { cod = cod*10 + (campoCod[i]-'0'); i++; }
        a.codigo = cod;

        leerCampo(linea, &pos, a.nombre,       MAX_NOMBRE);
        leerCampo(linea, &pos, a.tipo,         MAX_TIPO);
        leerCampo(linea, &pos, a.departamento, MAX_DEPTO);
        leerCampo(linea, &pos, a.responsable,  MAX_RESPONSABLE);
        leerCampo(linea, &pos, a.estado,       MAX_ESTADO);

        listaInsertarFinal(l, a); // cargar en orden del archivo
        total++;
    }
    fclose(f);
    printf("  [OK] %d activo(s) cargados desde '%s'.\n", total, ARCHIVO_DATOS);
}

// MODULO 5: Presentacion visual

void mostrarEncabezado(void) {
    printf("\n");
    printf("  +==========================================================+\n");
    printf("  |                                                            |\n");
    printf("  |     GESTION DE ACTIVOS TI  -  TechSolutions S.A.          |\n");
    printf("  |     Sistema Integral de Gestion de Servicios TI           |\n");
    printf("  |                                                            |\n");
    printf("  |     Programacion 1 - ISWZ1102   |   Grupo 4               |\n");
    printf("  |     Angel Mullo  &  Juyani Santillan                       |\n");
    printf("  |                                                            |\n");
    printf("  +==========================================================+\n");
}

void mostrarMenu(void) {
    printf("\n");
    printf("  +---------------- MENU PRINCIPAL ------------------+\n");
    printf("  |  [1]  Registrar activo                           |\n");
    printf("  |  [2]  Buscar activo                              |\n");
    printf("  |  [3]  Actualizar responsable                     |\n");
    printf("  |  [4]  Dar de baja activo                         |\n");
    printf("  |  [5]  Mostrar inventario completo                |\n");
    printf("  |  [6]  Mostrar activos por departamento           |\n");
    printf("  |  [7]  Ordenar inventario por codigo              |\n");
    printf("  |  [8]  Guardar datos                              |\n");
    printf("  |  [9]  Salir                                      |\n");
    printf("  +---------------------------------------------------+\n");
    printf("  Opcion: ");
}

// Trunca strings largos para que la tabla no se desalinee
static void truncar(const char *src, char *dst, int max) {
    int i = 0;
    while (src[i] != '\0' && i < max) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

// Imprime la ficha completa de un activo en formato de caja
static void imprimirFicha(Activo *a) {
    printf("  +--------------------------------------------+\n");
    printf("  | Codigo       : %-28d|\n", a->codigo);
    printf("  | Nombre       : %-28s|\n", a->nombre);
    printf("  | Tipo         : %-28s|\n", a->tipo);
    printf("  | Departamento : %-28s|\n", a->departamento);
    printf("  | Responsable  : %-28s|\n", a->responsable);
    printf("  | Estado       : %-28s|\n", a->estado);
    printf("  +--------------------------------------------+\n");
}

// Imprime la cabecera de la tabla del inventario
static void imprimirCabeceraTabla(void) {
    printf("\n  +-----------------------------------------------------------------+\n");
    printf("  | %-6s | %-18s | %-12s | %-15s | %-10s |\n",
           "Codigo", "Nombre", "Tipo", "Departamento", "Estado");
    printf("  +-----------------------------------------------------------------+\n");
}

// Imprime una fila del inventario con campos truncados
static void imprimirFilaTabla(Activo *a) {
    char nom[19], tip[13], dep[16], est[11];
    truncar(a->nombre,       nom, 18);
    truncar(a->tipo,         tip, 12);
    truncar(a->departamento, dep, 15);
    truncar(a->estado,       est, 10);
    printf("  | %-6d | %-18s | %-12s | %-15s | %-10s |\n",
           a->codigo, nom, tip, dep, est);
}

// MODULO 6: Operaciones del sistema

// Verifica si ya existe un activo con ese codigo en la lista
static int codigoExiste(Lista *l, int codigo) {
    return listaBuscarPorCodigo(l, codigo) != NULL;
}

// Muestra submenu de tipos y retorna el tipo seleccionado
static void seleccionarTipo(char *tipo) {
    printf("  Tipo de activo:\n");
    printf("    [1] PC/Laptop\n");
    printf("    [2] Servidor\n");
    printf("    [3] Red/Switch\n");
    printf("    [4] Impresora\n");
    printf("    [5] Otro\n");
    printf("  Opcion: ");
    int op = leerOpcion(1, 5);
    if (op == 1)      copiarStr(tipo, TIPO_PC);
    else if (op == 2) copiarStr(tipo, TIPO_SERVIDOR);
    else if (op == 3) copiarStr(tipo, TIPO_RED);
    else if (op == 4) copiarStr(tipo, TIPO_IMPRESORA);
    else              copiarStr(tipo, TIPO_OTRO);
}

// Registra un nuevo activo en la lista con codigo unico y estado "Activo"
void opRegistrar(Lista *l) {
    printf("\n=== REGISTRAR ACTIVO ===\n");
    Activo a;

    // Validar codigo unico antes de continuar
    do {
        a.codigo = leerEntero("  Codigo del activo: ", 1);
        if (codigoExiste(l, a.codigo))
            printf("  [!] Ese codigo ya existe. Ingrese otro.\n");
        else break;
    } while (1);

    leerTexto("  Nombre        : ", a.nombre, MAX_NOMBRE);
    sanearCampo(a.nombre);
    seleccionarTipo(a.tipo);
    leerTexto("  Departamento  : ", a.departamento, MAX_DEPTO);
    sanearCampo(a.departamento);
    leerTexto("  Responsable   : ", a.responsable, MAX_RESPONSABLE);
    sanearCampo(a.responsable);
    copiarStr(a.estado, ESTADO_ACTIVO); // todo activo nuevo comienza como Activo

    listaInsertarOrdenado(l, a); // insertar manteniendo orden por codigo
    archivoGuardar(l);
    printf("  [OK] Activo '%s' registrado exitosamente.\n", a.nombre);
}

// Busca un activo por codigo y muestra su ficha completa
void opBuscar(Lista *l) {
    printf("\n=== BUSCAR ACTIVO ===\n");
    int cod = leerEntero("  Codigo a buscar: ", 1);
    Nodo *n = listaBuscarPorCodigo(l, cod);
    if (n == NULL) { printf("  [!] No se encontro activo con codigo %d.\n", cod); return; }
    imprimirFicha(&n->dato);
}

// Actualiza el campo responsable de un activo (puntero in-place, sin mover el nodo)
void opActualizarResponsable(Lista *l) {
    printf("\n=== ACTUALIZAR RESPONSABLE ===\n");
    int cod = leerEntero("  Codigo del activo: ", 1);
    Nodo *n = listaBuscarPorCodigo(l, cod);
    if (n == NULL) { printf("  [!] Activo no encontrado.\n"); return; }

    printf("  Responsable actual: %s\n", n->dato.responsable);
    char nuevo[MAX_RESPONSABLE];
    leerTexto("  Nuevo responsable: ", nuevo, MAX_RESPONSABLE);
    sanearCampo(nuevo);
    copiarStr(n->dato.responsable, nuevo); // modificacion in-place usando puntero al nodo

    archivoGuardar(l);
    printf("  [OK] Responsable actualizado correctamente.\n");
}

// Cambia el estado del activo a "De Baja" (no se elimina fisicamente de la lista)
void opDarDeBaja(Lista *l) {
    printf("\n=== DAR DE BAJA ACTIVO ===\n");
    int cod = leerEntero("  Codigo del activo: ", 1);
    Nodo *n = listaBuscarPorCodigo(l, cod);
    if (n == NULL) { printf("  [!] Activo no encontrado.\n"); return; }

    if (strIguales(n->dato.estado, ESTADO_BAJA)) {
        printf("  [!] El activo ya esta dado de baja.\n");
        return;
    }

    printf("  Activo: [%d] %s\n", n->dato.codigo, n->dato.nombre);
    printf("  Confirmar baja (1=Si / 2=No): ");
    if (leerOpcion(1, 2) == 2) { printf("  [!] Operacion cancelada.\n"); return; }

    copiarStr(n->dato.estado, ESTADO_BAJA); // cambiar estado in-place
    archivoGuardar(l);
    printf("  [OK] Activo dado de baja correctamente.\n");
}

// Muestra todos los activos de la lista en formato de tabla
void opMostrarInventario(Lista *l) {
    if (l->cantidad == 0) { printf("\n  [!] El inventario esta vacio.\n"); return; }

    printf("\n=== INVENTARIO COMPLETO (%d activo(s)) ===", l->cantidad);
    imprimirCabeceraTabla();

    Nodo *actual = l->cabeza;
    while (actual != NULL) {
        imprimirFilaTabla(&actual->dato);
        actual = actual->siguiente; // recorrido hacia adelante
    }
    printf("  +-----------------------------------------------------------------+\n");
}

// Filtra y muestra activos de un departamento especifico (busqueda case-insensitive)
void opMostrarPorDepartamento(Lista *l) {
    if (l->cantidad == 0) { printf("\n  [!] El inventario esta vacio.\n"); return; }

    char depto[MAX_DEPTO];
    leerTexto("\n  Departamento a consultar: ", depto, MAX_DEPTO);

    int encontrados = 0;
    printf("\n=== ACTIVOS DEL DEPARTAMENTO: %s ===", depto);
    imprimirCabeceraTabla();

    Nodo *actual = l->cabeza;
    while (actual != NULL) {
        // strIguales permite buscar "TI" aunque este guardado como "ti"
        if (strIguales(actual->dato.departamento, depto)) {
            imprimirFilaTabla(&actual->dato);
            encontrados++;
        }
        actual = actual->siguiente;
    }
    printf("  +-----------------------------------------------------------------+\n");
    if (encontrados == 0)
        printf("  [!] No se encontraron activos para el departamento '%s'.\n", depto);
    else
        printf("  Total encontrados: %d\n", encontrados);
}

// Ordena la lista por codigo y guarda el nuevo orden en el archivo
void opOrdenar(Lista *l) {
    if (l->cantidad < 2) { printf("\n  [i] No hay suficientes activos para ordenar.\n"); return; }
    listaOrdenarPorCodigo(l);  // algoritmo burbuja sobre los datos de los nodos
    archivoGuardar(l);
    printf("\n  [OK] Inventario ordenado por codigo. Total: %d activo(s).\n", l->cantidad);
    opMostrarInventario(l);    // mostrar resultado del ordenamiento
}

// MODULO 7: Menu principal e integracion

int main() {
    Lista inventario; // lista principal: vive en main() y se pasa por puntero
    listaInicializar(&inventario);

    mostrarEncabezado();
    archivoCargar(&inventario); // cargar datos previos al iniciar

    int opcion;
    do {
        mostrarMenu();
        opcion = leerOpcion(1, 9);

        // Cada funcion recibe &inventario (puntero) para modificar la lista por referencia
        switch (opcion) {
            case 1: opRegistrar(&inventario);              break;
            case 2: opBuscar(&inventario);                 break;
            case 3: opActualizarResponsable(&inventario);  break;
            case 4: opDarDeBaja(&inventario);              break;
            case 5: opMostrarInventario(&inventario);      break;
            case 6: opMostrarPorDepartamento(&inventario); break;
            case 7: opOrdenar(&inventario);                break;
            case 8: archivoGuardar(&inventario);
                    printf("  [OK] Datos guardados en '%s'.\n", ARCHIVO_DATOS); break;
            case 9:
                printf("\n  Guardando y cerrando sistema...\n");
                archivoGuardar(&inventario); // guardado final de seguridad
                break;
        }
    } while (opcion != 9);

    listaLiberar(&inventario); // liberar toda la memoria dinamica al salir
    printf("  Hasta pronto. - TechSolutions S.A.\n\n");
    return 0;
}
