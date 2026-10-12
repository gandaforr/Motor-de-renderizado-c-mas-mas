#include <cmath> // Necesario para usar cos() y M_PI (o acos)
#include <random> //para numeros random

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

    static Vector LocalAGlobal(Vector w, Vector local){
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
        return (u * local.x + v * local.y + w * local.z).normalize();
    }
	bool soyCero(){return x==0 && y==0 && z==0;}
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

// Retorna un número aleatorio en el rango [min, max)
inline double randomDouble(double min = 0.0, double max = 1.0) {
    // thread_local inicializa un generador independiente por cada hilo (crucial si paralelizas el render)
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<double> distribution(min, max);
    
    return distribution(generator);
}

// Xorshift32: extremadamente rápido y ligero
// no note mucha diferencia, y me lo dio con ruido omg no
inline double randomDoubleFast() {
    static thread_local uint32_t state = 123456789; // Semilla por hilo
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state * (1.0 / 4294967296.0); // Divide entre 2^32
}