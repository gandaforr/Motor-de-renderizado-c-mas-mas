# 🎨 C++ Ray Tracer & Rendering Engine (Monte Carlo & BRDF)

![C++](https://img.shields.io/badge/Language-C%2B%2B14%2F17-blue.svg)
![Graphics](https://img.shields.io/badge/Domain-Computer_Graphics_%2F_Ray_Tracing-red.svg)
![Design Pattern](https://img.shields.io/badge/Pattern-Decorator-brightgreen.svg)

Motor de renderizado y trazado de rayos (*Ray Tracer*) construido desde cero en **C++ moderno**. El sistema modela la interacción física de la luz sobre geometrías 3D utilizando **Integración de Monte Carlo** para estimación de iluminación y modelos de reflectancia **BRDF difusa**, exportando el resultado en imágenes de formato **PPM**.

---

## 📌 Aspectos Técnicos Destacados

* **Ray Tracing Basado en Física:** Cálculo numérico de intersecciones Rayo-Geometría (`intersect`), cálculo de vectores normales y sombreado continuo.
* **Integración por Monte Carlo:** Muestreo estocástico (*stochastic sampling*) para la estimación de iluminación global, cálculo de sombreado y suavizado de sombras.
* **Iluminación (BRDF):** Implementación de funciones de distribución de reflectancia bidireccional (*Bidirectional Reflectance Distribution Function*) para simular materiales difusos e intensidad lumínica.
* **Arquitectura Orientada a Objetos y Patrones de Diseño:**
  * **Patrón Decorador (Decorator Pattern):** Permite añadir dinámicamente comportamientos físicos (como propiedades de luz, BRDF o muestreo de Monte Carlo) envolviendo instancias de geometrías base (`Figura`).
  * **Gestión Eficiente de Memoria (Modern C++):** Uso estricto de punteros inteligentes (`std::unique_ptr`) y semántica de movimiento (`std::move`) para transferencias de propiedad sin *memory leaks*.
* **Exportador PPM:** Renderizado directo a formato de imagen sin dependencia de librerías gráficas pesadas de terceros.

---

## 🏗️ Patrón de Diseño & Arquitectura

El motor utiliza una estructura polimórfica extensible basada en el patrón **Decorator**:
```mermaid
graph TD
    Figura["<b>Figura</b><br/><i>(Clase Abstracta / Interfaz)</i>"]
    Esfera["<b>Esfera</b><br/><i>(Geometría Base)</i>"]
    Luz["<b>Luz</b><br/><i>(Decorador Base)</i><br/><code>std::unique_ptr&lt;Figura&gt;</code>"]
    MonteCarlo["<b>conMonteCarlos</b><br/><i>(Muestreo estocástico)</i>"]
    BRDF["<b>conBRDFdifusa</b><br/><i>(Propiedades ópticas)</i>"]

    Figura -.->|Implementa| Esfera
    Figura --->|Herencia| Luz
    Luz --->|Decora| MonteCarlo
    MonteCarlo --->|Decora| BRDF
    Luz -.->|Envuelve| Figura
```
### Ejemplo de Composición Dinámica
Mediante el uso de `std::move`, el sistema "decora" dinámicamente un objeto básico:

// 1. Instancia base
std::unique_ptr<Figura> miEsf = std::make_unique<Esfera>(r, p, c);

// 2. Decoración con muestreo estocástico (100 muestras)
miEsf = std::make_unique<conMonteCarlos>(std::move(miEsf), 100);

// 3. Decoración con BRDF Difusa
miEsf = std::make_unique<conBRDFdifuda>(std::move(miEsf), intensidad); 

## Visualización del resultado
<img width="1024" height="768" alt="ece2(4)" src="https://github.com/user-attachments/assets/f35021d4-860c-450d-9dbe-0fc13f9f8efb" />

## Autor
Odilón Avila Castrejón — Estudiante de Ingeniería en Computación (UMSNH) 

LinkedIn: https://www.linkedin.com/in/odil%C3%B3n-avila-castrejon-484757384/
