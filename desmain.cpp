#include <math.h>
#include <stdlib.h>
#include <stdio.h>  
#include <omp.h>
#include <fstream>
//para los punteros inteligentes [los que no necesito borrar manualmente segun gemini]
#include <memory>
#include <vector>
// para emular los diccionarios de python
// me preocupa un poco agregar tanta libreria
// se canselo eso de emular el diccionario de python
// encontre otra forma mas al estilo c++
#include <unordered_map>
#include <any> // Permite guardar cualquier tipo de dato de forma segura
#include <string> // el string si lo use

// tiene el min
#include <algorithm>
// me acabo ddar cuenta que se me olvido poner el
// using namespace std; ;_;

#include <cstdlib> // Para rand() y srand()
#include <ctime>   // Para time()

#include <cmath> // Necesario para usar cos() y M_PI (o acos)

using namespace std;

class Escenario;
class Luz;
class Figura;

// en el anterior separe el codigo en varios archivos
// lo pense como hacerlo de la forma mas profesional,
// esta vez lo pondre todo en el mismo lugar 
// creo que incluso complique la lectura al separarlo asi
// en el momento no me molesto por que sabia donde estaba todo
// pero ahora me cuesta encontrar ciertas cosas

class Vector 
{
public:        
	double x, y, z; // coordenadas x,y,z 
  
	// Constructor del vector, parametros por default en cero
	Vector(double x_= 0, double y_= 0, double z_= 0){ x=x_; y=y_; z=z_; }
  
	// operador para suma y resta de vectores
	Vector operator+(const Vector &b) const { return Vector(x + b.x, y + b.y, z + b.z); }
	Vector operator-(const Vector &b) const { return Vector(x - b.x, y - b.y, z - b.z); }
	// operator multiplicacion vector y escalar 
	Vector operator*(double b) const { return Vector(x * b, y * b, z * b); }
  
	// operator % para producto cruz
	Vector operator%(Vector&b){return Vector(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);}

	Vector operator/(double b){return Vector(x/b, y/b, z/b);}
	
	// producto punto con vector b
	double dot(const Vector &b) const { return x * b.x + y * b.y + z * b.z; }

	// producto elemento a elemento (Hadamard product)
	Vector mult(const Vector &b) const { return Vector(x * b.x, y * b.y, z * b.z); }
	
	// normalizar vector 
	Vector& normalize(){ return *this = *this * (1.0 / sqrt(x * x + y * y + z * z)); }
};
typedef Vector Point;
typedef Vector Color;
// el vector tambien llamado point y color en el codigo
class Ray 
{ 
public:
	Point o;
	Vector d; // origen y direcccion del rayo
	Ray(Point o_, Vector d_) : o(o_), d(d_) {} // constructor
};

// agregado para quitar la circularidad que meti por no fiharme
class Intersecto{
	public:
		virtual bool intersect(const Ray &r, double &t, int &id) = 0;
		virtual ~Intersecto() = default; // por si acaso
};



// pondre la logica de las figuras abajo de la intefaz/clase abstracta 
// correspondiente
class Figura{
	public:
		virtual double intersect(const Ray &ray) const = 0; 
		virtual ~Figura() = default;
		virtual std::unique_ptr<Luz>& getIluminacion() = 0;
		virtual Vector normalN(Point x) = 0;
		virtual Color getColor() = 0;
};


class Esfera : public Figura{
	private:
		Color c;
		Point p;
		double r;
		// si me faltaba el using namespace std por eso no me dejaba antes, en el ecenario
	public:
		unique_ptr<Luz> iluminacion;
		Esfera(double _r,Point _p, Color _c) : p(_p), c(_c), r(_r) {}
		Esfera(std::unique_ptr<Luz> _iluminacion,double _r, Point _p, Color _c) : iluminacion(move(_iluminacion)),  p(_p), c(_c), r(_r) {}

		// exactamente lo que tenia en el anterior
		double intersect(const Ray &ray) const override {
			//lo de afuera
			Vector julio;
			double julia,julini,dentro,v1,v2;
			julio = ray.o.operator-(this->p);
			julia = julio.dot(ray.d);
			julini = julia*-1;
			
			//--------------------// //E
			//double epsilon = 20;
			dentro = sqrt(pow(julia,2)- julio.dot(julio) + pow(this->r,2));
			v1 = julini - dentro;
			v2 = julini + dentro;

			
			// regresar distancia si hay interseccion
			if(v1>=0 && v2>=0){
				return std::min(v1,v2);
			}
			else if(v1>=0){
				return v1;
			}
			else if(v2>=0){
				return v2;
			}
			//else if(v1+epsilon >= 0 || v2+epsilon >= 0){
			//	return max(v1,v2) + epsilon;
			//}
		
			// regresar 0.0 si no hay interseccion
			return 0.0;
		}

		Color getColor() override{
			return c;
		}

		Vector normalN(Point x) override{
			return p - x;
		}

		std::unique_ptr<Luz>& getIluminacion() override{
			return iluminacion;
		}
};

// en algun futuro agregar la figura punto



// la logica de la Luz
class Luz{
	public:
		// ese ecenario hay me incomoda un poco, ya lo quite me dio problemas
		virtual Color shade(const Ray &r, double t, int id,Figura &figura,Intersecto &entorno) const = 0;
		virtual ~Luz() = default;
};
class SimpleLuz: public Luz{
	public:
		Color shade(const Ray &r, double t, int id,Figura &figura,Intersecto &entorno) const override {
			return figura.getColor();
		}
};

class luzMonte: public Luz{
	public:
		int n;
		luzMonte(int n_muestras, int mi_id) : n(n_muestras) {}

		Color shade(const Ray &r, double t, int id,Figura &figura,Intersecto &entorno) const override {
			Point x = r.o +  r.d*t; 
			Vector normal = figura.normalN(x);
			double r1,r2,phi,t_aux;
			int id_aux;
			Color c(0,0,0);
			for(int i=0;i<n;i++){
				r1 = (double)rand() / RAND_MAX;
				r2 = (double)rand() / RAND_MAX;
				phi = 2.0 * M_PI * r2;

				Vector ww(cos(phi)*sqrt(r1), sin(phi)*sqrt(r1),sqrt(1 - r1));
				// Construcción optimizada de la base local (PBRT / Hughes-Möller)
				Vector w = normal;
				Vector u;
				Vector v;
				if (std::abs(w.x) > std::abs(w.y)) {
				double invLen = 1.0 / std::sqrt(w.x * w.x + w.z * w.z);
				u = Vector(w.z * invLen, 0.0, -w.x * invLen);
				} else {
				double invLen = 1.0 / std::sqrt(w.y * w.y + w.z * w.z);
				u = Vector(0.0, w.z * invLen, -w.y * invLen);
				}
				v = u % w; // Producto cruz para completar la base ortogonal

				//Vector wi = (u * x_local + v * y_local + w * z_local).normalize();
				Vector wi = (u * ww.x + v * ww.y + w * ww.z).normalize();
				Ray r_aux(x + x*0.1,wi.normalize());
				entorno.intersect(r_aux,t_aux,id_aux);

				if(id_aux == id){
					double cos_teta = normal.dot(wi);
					c = c + figura.getColor()/M_PI * (cos_teta * cos_teta/M_PI);
				}

				

			}
		return c/n;			
		}
};


/*
struct rtid{
	Ray &r;
	double t;
	int id;
};
*/



// usare estructuras con todos los parametros posibles para
// crear figuras

struct v3{ float x=0,y=0,z=0; };
struct Params {
    std::string tipo;
    v3 p{};
    v3 z{};
	double radio=0;
};


inline double clamp(const double x) { 
	if(x < 0.0)
		return 0.0;
	else if(x > 1.0)
		return 1.0;
	return x;
}

inline int toDisplayValue(const double x) {
	return int( pow( clamp(x), 1.0/2.2 ) * 255 + .5); 
}

class Escenario : public Intersecto{
	private:
		// aqui si tuve que poner el std 
		//ya vi por que ;_;
		
		std::vector<Figura*> luces;
		Ray &camera;
		int h, w;

		// me estoy debatiendo en hacer una clase camara por los comentario
		// del codigo que nos paso el profe al decir que cx y cy, son parametros
		// de la camara

	public:
		std::vector<std::unique_ptr<Figura>> figuras;
    	Escenario(Ray &c, int height, int width) : camera(c), h(height), w(width){}

		// tal vez lo pase a privado en un futuro ---creo que ya no,
		// solo es necesario dentro de la clase
		// pero ya estoy acostumbrado a poner los metodos publico
		bool intersect(const Ray &r, double &t, int &id) override{
			t = 0;
			double distancia = 0;
			int i = 0;
			for (const auto& figura : figuras){
				distancia = figura->intersect(r);
				if (distancia > 0.001 && (t==0 || distancia < t)){
					t = distancia;
					id = i;
				}
				i = i+1;
			}
			/*
			el del anterior proyecto
			for (size_t i = 0; i < escenario.objetos.size(); i++) {
				double bb = escenario.objetos[i]->intersect(r);
				if (bb > 0.001 && (t == 0 || bb < t)) {
					t = bb;
					id = i;
				}
			}
			return t != 0;
			*/
			return t != 0;
		}

		// puede que despues le cambie el nombre a uno mas apropiado
		Color star(const Ray &r){
			double t;
			int id;
			if(!intersect(r,t,id)){
				return Color(0,0,0);
			}

			Color c(0,0,0);
			for (Figura* luz : luces){
				if(luz != nullptr){
					const auto& a = luz->getIluminacion();
					c = c + a->shade(r,t,id,*figuras[id].get(),*this);
				}
			}
			return c;
		}

		void renderizar(const string &filename){
			// aqui es donde se va a crear la imagen
			// en el proyecto anterior que hice de graficacion
			// esta parte impresionantemente nunca cambio
			// siempre fue el mismo de principio a fin
			// pero aqui lo voy a modificar tantito
			// para adaptarlo al nuevo diseño
			Vector cx = Vector(w * 0.5095 / h, 0., 0.);
			Vector cy = (cx % camera.d).normalize() * 0.5095;
			
			//me sienta mal poner el puntero asi pero queda bien
			Color *pixelColors = new Color[w * h];
			
			#pragma omp parallel for schedule(dynamic, 1)
			for (int y = 0; y < h; y++) {
				fprintf(stderr, "\rRenderizando - %5.2f%%", 100.0 * y / (h - 1));
				//Shade copiarPorHilo = shade;
				for (int x = 0; x < w; x++) {
					int idx = (h - y - 1) * w + x;
					Vector cameraRayDir = cx * (double(x) / w - 0.5) +
										cy * (double(y) / h - 0.5) + camera.d;
					
					
					Color pixelValue = star(Ray(camera.o, cameraRayDir.normalize()));
					//Color pixelValue = copiarPorHilo.star(Ray(camera.o, cameraRayDir.normalize())); no funciono ;_; me dio error
					//cout<<"\nEn el render\n";
					//pixelValue.muestrate();
					//cout<<endl;
					
					pixelColors[idx] = Color(clamp(pixelValue.x), clamp(pixelValue.y), clamp(pixelValue.z));
				}
			}

				fprintf(stderr,"\n");

			FILE *f = fopen(filename.c_str(), "w");
			// escribe cabecera del archivo ppm, ancho, alto y valor maximo de color
			fprintf(f, "P3\n%d %d\n%d\n", w, h, 255); 
			for (int p = 0; p < w * h; p++) 
			{ // escribe todos los valores de los pixeles
					fprintf(f,"%d %d %d ", toDisplayValue(pixelColors[p].x), toDisplayValue(pixelColors[p].y), 
						toDisplayValue(pixelColors[p].z));
			}
			fclose(f);

			delete[] pixelColors;

		}

		// reglas para agregar cosas al ecenario
			// tanto la luces y las formas geometricas ambas
			// son figuras en la abstraccion que hice, para
			// diferenciar de una figura que ilumina de una que no
			// decidi que la diferencia hiba a estar en una lista.
			// en la lista de figuras hay van estar todas las figuras
			// incluyendo las que iluminan, y en luces solo van a estar
			// las figuras que iluminan, para lograr esto voy hacer que
			// las figuras se creen desde el ecenario para asegurarme
			// de que la regla se cumpla 

		void agregar_figura(const Params& params){
			// le digo la figura y sus parametros, en plan figura=esfera, p=(z,x,y), c=(z,x,y)
			// lo puedo pasar como un pz=1,px=2,py=4, cz=1,cx=3,cy=2 pero es muy largo de escribir

			// descubri otra forma usando struct
			if(params.tipo == "predeterminado1"){
				//SimpleLuz laIluminacion; no ucurrio
				figuras.push_back(std::make_unique<Esfera>(1e5,  Point(-1e5 - 49, 0, 0),   Color(.75, .25, .25))); // pared izquierda
				figuras.push_back(std::make_unique<Esfera>(1e5,  Point(1e5 + 49, 0, 0),    Color(.25, .25, .75))); // pared derecha
				figuras.push_back(std::make_unique<Esfera>(1e5,  Point(0, 0, -1e5 - 81.6), Color(.75, .75, .75))); // pared  detras
				figuras.push_back(std::make_unique<Esfera>(1e5,  Point(0, -1e5 - 40.8, 0), Color(.75, .75, .75))); // suelo
				figuras.push_back(std::make_unique<Esfera>(1e5,  Point(-1e5 - 49, 0, 0),   Color(.75, .25, .25))); // techo
				figuras.push_back(std::make_unique<Esfera>(16.5, Point(-23, -24.3, -34.6), Color(.999, .999, .999))); // esfera abajo-izq
				figuras.push_back(std::make_unique<Esfera>(16.5, Point(23, -24.3, -3.6),   Color(.999, .999, .999))); // esfera abajo-der
				figuras.push_back(std::make_unique<Esfera>(std::make_unique<SimpleLuz>(),10.5, Point(0, 24.3, 0),        Color(1, 1, 1))); // esfera arriba
				luces.push_back(figuras.back().get());
			}
			else if (params.tipo == "predeterminado2"){
				figuras.push_back(std::make_unique<Esfera>(1e5,  Point(-1e5 - 49, 0, 0),   Color(.75, .25, .25))); // pared izquierda
				figuras.push_back(std::make_unique<Esfera>(1e5,  Point(1e5 + 49, 0, 0),    Color(.25, .25, .75))); // pared derecha
				figuras.push_back(std::make_unique<Esfera>(1e5,  Point(0, 0, -1e5 - 81.6), Color(.75, .75, .75))); // pared  detras
				figuras.push_back(std::make_unique<Esfera>(1e5,  Point(0, -1e5 - 40.8, 0), Color(.75, .75, .75))); // suelo
				figuras.push_back(std::make_unique<Esfera>(1e5,  Point(-1e5 - 49, 0, 0),   Color(.75, .25, .25))); // techo
				figuras.push_back(std::make_unique<Esfera>(16.5, Point(-23, -24.3, -34.6), Color(.999, .999, .999))); // esfera abajo-izq
				figuras.push_back(std::make_unique<Esfera>(16.5, Point(23, -24.3, -3.6),   Color(.999, .999, .999))); // esfera abajo-der
				figuras.push_back(std::make_unique<Esfera>(std::make_unique<luzMonte>(100),10.5, Point(0, 24.3, 0),        Color(1, 1, 1))); // esfera arriba
				luces.push_back(figuras.back().get());
			}

		}
		void agregar_iluminacion(){

		}
};

int main(){
	srand(time(nullptr));
    Ray camera( Point(0, 11.2, 214), Vector(0, -0.042612, -1).normalize() );
	//Ray camera( Point(0, 11.2, 214), Vector(0, 0, 0).normalize() );
	// posicion de origen, posicion camara
	//  (camara - origen).normalize() => nueva direccion
	Escenario huevos_de_gamboa(camera, 768, 1024);
	huevos_de_gamboa.agregar_figura({.tipo="predeterminado2"});
	huevos_de_gamboa.renderizar("parte2-1.ppm");
}

/*
inline
1. El significado original: Sustitución de código (Optimización)Tradicionalmente, inline le sugiere al compilador que, en lugar de hacer una "llamada" formal a la función (lo que implica pausar la ejecución actual, saltar a otra parte de la memoria, ejecutar y regresar), copie y pegue el cuerpo de la función directamente en el lugar donde la llamaste.Sin inline: El programa salta a la función. Esto toma un tiempo mínimo, pero si la función se llama millones de veces dentro de un bucle, puede ralentizar el programa.Con inline: El compilador expande el código. Elimina el costo del salto, haciendo que el programa sea más rápido, a costa de que el archivo ejecutable final sea un poco más grande en tamaño.
Nota moderna: Hoy en día, los compiladores son sumamente inteligentes. El compilador decidirá si "pega" el código o no basándose en el rendimiento, sin importar si tú escribiste la palabra inline o no.

2. El significado moderno: La Regla de Definición Única (ODR)Este es el verdadero motivo por el cual los programadores usan inline hoy en día.En C++, por regla general (llamada One Definition Rule), no puedes definir la misma función más de una vez en tu programa. Si pones una función normal con todo su cuerpo dentro de un archivo de cabecera (.h o .hpp) y ese archivo es incluido (#include) en varios archivos .cpp, el compilador se quejará de un error de "definición duplicada".Al marcar una función como inline, le estás diciendo al compilador: "Esta función va a aparecer en múltiples archivos de traducción, por favor permítelo y no lances un error de duplicados. Al final, unifica todas las definiciones en una sola".
Dato curioso: Si defines una función dentro de la declaración de una clase (muy común en C++ y Java), el compilador automáticamente la considera inline sin necesidad de que escribas la palabra.

*/

// pensamiento lo modificare despues talvez esa relacion entre la figura y la luz
// descubri que si quito la funcion esa para acceder al metodo shade/iluminar 
// de la luz, quito esa relacion rara ahora modificar la luz solo altera la luz
// y alterar la figura solo altera a la figura
// 
// aun tengo que modificar la relacion de ecenario-luz
// se tubo que modificar para que compilara, habia creado un circulo


/*


*/