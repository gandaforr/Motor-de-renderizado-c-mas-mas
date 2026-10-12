#include "base.cpp"
//para punteros inteligentes y strings
#include <string>
#include <memory>

// para las listas
#include <vector>

// para numeros randoms
#include <cstdlib> // Para rand() y srand()
#include <ctime>   // Para time()

using namespace std;

class Figura{
    public:
        virtual double intersect(const Ray &ray) const = 0; 
        virtual Ray getRayEnSuperficie(Point x,Vector &n) = 0;
        virtual Color getColor(const Ray &r, double t, int id ) = 0;
        virtual Color shade(Color c, Vector wi, Vector n) = 0;
        virtual bool soyLuz() = 0;
		virtual ~Figura() = default;
};

class Luz: public Figura{
    protected:
        std::unique_ptr<Figura> figuraBase;
    public:
    Luz(std::unique_ptr<Figura> figura) 
        : figuraBase(std::move(figura)) {}
};
class Esfera : public Figura{
    private:
		Color c;
		Point p;
		double r;
    public:
        Esfera(double _r,Point _p, Color _c) : p(_p), c(_c), r(_r) {}
        double intersect(const Ray &ray) const override {
			Vector julio;
			double julia,julini,dentro,v1,v2;
			julio = ray.o.operator-(this->p);
			julia = julio.dot(ray.d);
			julini = julia*-1;

			dentro = sqrt(pow(julia,2)- julio.dot(julio) + pow(this->r,2));
			v1 = julini - dentro;
			v2 = julini + dentro;

			if(v1>=0 && v2>=0){
				return std::min(v1,v2);
			}
			else if(v1>=0){
				return v1;
			}
			else if(v2>=0){
				return v2;
			}

			return 0.0;
		}

        Color getColor(const Ray &r, double t, int id) override{
            return c;
        }

        Ray getRayEnSuperficie(Point x, Vector &n) override{
            double r1 = randomDouble(); //pues no noto tanta diferencia
            double r2 = randomDouble();
            double phi = 2.0 * M_PI * r2;
            n = (x - this->p).normalize();
            Vector local(cos(phi)*sqrt(r1),sin(phi)*sqrt(r1),sqrt(1.0-r1));
            Vector wi = Vector::LocalAGlobal(n,local);
            return Ray(x,wi);

        }

        bool soyLuz() override{
            return false;
        }

        Color shade(Color c, Vector wi, Vector n) override{
            return Color(0,0,0);
        }

};

// ecenario
std::vector<std::unique_ptr<Figura>> figuras; 
// si lo quiero pasar como variable const std::vector<std::unique_ptr<Figura>>& ecenario
// es una gramatica muy larga, se pasa r,t,id
bool globalIntersect(const Ray &r, double &t, int &id){
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
    return t!=0;
}

class conMonteCarlos : public Luz{
    public:
        int n;
        conMonteCarlos(std::unique_ptr<Figura> figura, int _n) : Luz(std::move(figura)), n(_n) {}
        double intersect(const Ray &ray) const override{return figuraBase->intersect(ray);}
        Ray getRayEnSuperficie(Point x, Vector &n) override{return figuraBase->getRayEnSuperficie(x,n);}
        Color shade(Color c, Vector wi, Vector n) override {return figuraBase->shade(c,wi,n);}
        bool soyLuz() override {return figuraBase->soyLuz();}
        Color getColor(const Ray &r, double t, int id) override{
            Color c = figuraBase->getColor(r,t,id);
            if(figuras[id]->soyLuz()){
                return c;
            }
            Color g(0,0,0);
            Point x = r.o +  r.d*t;
            Vector nor(0,0,0);
            for(int i=0;i<n;i++){
                Ray nr = this->getRayEnSuperficie(x,nor);
                int auxid;
                double auxt;
                if(globalIntersect(nr,auxt,auxid)){
                    g = g + figuras[auxid]->shade(c,nr.d,nor);
                }
            }
            return g/n;
        }
};

class conBRDFdifuda : public Luz{
    public:
        Vector intensidad;
        conBRDFdifuda(std::unique_ptr<Figura> figura, Vector _intensidad) : Luz(std::move(figura)), intensidad(_intensidad) {}
        double intersect(const Ray &ray) const override{return figuraBase->intersect(ray);}
        Ray getRayEnSuperficie(Point x, Vector &n) override{return figuraBase->getRayEnSuperficie(x,n);}
        Color shade(Color c, Vector wi, Vector n) override {
            double ndotwi = n.dot(wi);
            return intensidad.mult(c/M_PI) * (ndotwi/(ndotwi/M_PI));
        }
        bool soyLuz() override {return true;}
        Color getColor(const Ray &r, double t, int id) override{return figuraBase->getColor(r,t,id);}
};

Color star(const Ray &r){
    double t;
	int id;
	if(!globalIntersect(r,t,id)){
		return Color(0,0,0);
	}
    return figuras[id]->getColor(r,t,id);
}

void renderizar(const string &filename,Ray camera,int w, int h){
    Vector cx = Vector(w * 0.5095 / h, 0., 0.);
    Vector cy = (cx % camera.d).normalize() * 0.5095;
    
    Color *pixelColors = new Color[w * h];
    
    #pragma omp parallel for schedule(dynamic, 3)
    for (int y = 0; y < h; y++) {
        fprintf(stderr, "\rRenderizando - %5.2f%%", 100.0 * y / (h - 1));
        //Shade copiarPorHilo = shade;
        for (int x = 0; x < w; x++) {
            int idx = (h - y - 1) * w + x;
            Vector cameraRayDir = cx * (double(x) / w - 0.5) +
                                cy * (double(y) / h - 0.5) + camera.d;
            
            
            Color pixelValue = star(Ray(camera.o, cameraRayDir.normalize()));
            
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

void esferaMonteCarlos(double r, Point p, Color c,Vector intensidad = Vector(0,0,0)){
    std::unique_ptr<Figura> miEsf = std::make_unique<Esfera>(r,p,c);
    miEsf = std::make_unique<conMonteCarlos>(std::move(miEsf),100);
    if (!intensidad.soyCero()){miEsf = std::make_unique<conBRDFdifuda>(std::move(miEsf),intensidad);}
    figuras.push_back(std::move(miEsf));
}

void crearEcenario2(){
    esferaMonteCarlos(1e5,  Point(-1e5 - 49, 0, 0),   Color(.75, .25, .25));
    esferaMonteCarlos(1e5,  Point(1e5 + 49, 0, 0),    Color(.25, .25, .75));
    esferaMonteCarlos(1e5,  Point(0, 0, -1e5 - 81.6), Color(.75, .75, .75));
    esferaMonteCarlos(1e5,  Point(0, -1e5 - 40.8, 0), Color(.75, .75, .75));
    esferaMonteCarlos(1e5,  Point(0, 1e5 + 40.8, 0),  Color(.75, .75, .75));
    esferaMonteCarlos(16.5, Point(-23, -24.3, -34.6), Color(.999, .999, .999));
    esferaMonteCarlos(16.5, Point(23, -24.3, -3.6),   Color(.999, .999, .999));
    esferaMonteCarlos(10.5, Point(0, 24.3, 0),        Color(1, 1, 1), Vector(5,5,5));
}

int main(){
    srand(time(nullptr));
    crearEcenario2();
    Ray camera( Point(0, 11.2, 214), Vector(0, -0.042612, -1).normalize() );
    renderizar("bolas_sombreadas.ppm",camera,1024,768);
    //renderizar("4k.ppm",camera,2240,1280);
}