/*
Copyright (C) 2006 BEYLER SCHWARTZ Pierre

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <map>
#include <list>
#include <utility>
#include "noeud.h"

using namespace std;

sf::Image s;


typedef map< pair<int,int>, noeud> l_noeud;

l_noeud liste_ouverte;
l_noeud liste_fermee;
list<sf::Vector2u> chemin;
list<sf::Vector2u> passage;

sf::Vector2u arrivee;
noeud depart;

//float distance(int, int, int, int);
void ajouter_cases_adjacentes(pair<int,int>&);
bool deja_present_dans_liste( pair<int,int>,
                              l_noeud&);
pair<int,int> meilleur_noeud(l_noeud&);
void ajouter_liste_fermee(pair<int,int>&);
void ecrire_bmp(bool);
void colorerPixel(int, int, sf::Uint32);
void retrouver_chemin();
void definir_points_clees();
pair<int,int> trouver_un_point(sf::Color);
bool tracer_droite(pair<int,int>&, pair<int,int>&);
void ecrire_bmp_droite();


int main(int argc, char **argv){
	if(!s.loadFromFile("big.bmp")){
        cerr << "erreur lors du chargement du bmp de d�part" << endl;
        return -1;
	}

    pair<int,int> point = trouver_un_point(sf::Color::Green);

    depart.parent = point;

    point = trouver_un_point(sf::Color::Blue);

    if((point.first == 0) && (point.second == 0))
    {
        arrivee.x = s.getSize().x - 1;
        arrivee.y = s.getSize().y - 1;
    }
    else
    {
        arrivee.x = point.first;
        arrivee.y = point.second;
    }

    pair <int,int> courant;

    courant = depart.parent;

    bool cond = tracer_droite(depart.parent, point);

    if(!cond)
    {
        // d�roulement de l'algo A*
        // ajout de courant dans la liste ouverte

        liste_ouverte[courant]=depart;
        ajouter_liste_fermee(courant);
        ajouter_cases_adjacentes(courant);


        while( !((courant.first == arrivee.x) && (courant.second == arrivee.y))
                &&
            (!liste_ouverte.empty())
            ){

            // on cherche le meilleur noeud de la liste ouverte, on sait qu'elle n'est pas vide donc il existe
            courant = meilleur_noeud(liste_ouverte);

            // on le passe dans la liste fermee, il ne peut pas d�j� y �tre
            ajouter_liste_fermee(courant);

            ajouter_cases_adjacentes(courant);
            }
    }
    if(cond)
    {
        passage.push_back(arrivee);
        s.setPixel(arrivee.x, arrivee.y,sf::Color::Red);
        ecrire_bmp(cond);
    }

    if ((courant.first == arrivee.x) && (courant.second == arrivee.y)){
        retrouver_chemin();
        ecrire_bmp(cond);
    }else{
        // pas de solution
    }
    return 0;
}

pair<int,int> trouver_un_point(sf::Color c)
{
    for (int i=0; i<s.getSize().x; i++){
        for (int j=0; j<s.getSize().x; j++){
            if (s.getPixel(i, j) == c)
            {
                return pair<int,int> (i,j);
            }
        }
    }
    return pair<int,int> (0,0);
}

/*
ajoute toutes les cases adjacentes � n dans la liste ouverte
*/
void ajouter_cases_adjacentes(pair <int,int>& n){
    noeud tmp;

    // on met tous les noeud adjacents dans la liste ouverte (+v�rif)
    for (int i=n.first-1; i<=n.first+1; i++){
        if ((i<0) || (i>=s.getSize().x))
            continue;
        for (int j=n.second-1; j<=n.second+1; j++){
            if ((j<0) || (j>=s.getSize().y))
                continue;
            if ((i==n.first) && (j==n.second))  // case actuelle n
                continue;

            if (s.getPixel(i, j) == sf::Color::Black)
                // obstace, terrain non franchissable
                continue;

            pair<int,int> it(i,j);

            if (!deja_present_dans_liste(it, liste_fermee)){
                // le noeud n'est pas d�j� pr�sent dans la liste ferm�e

                tmp.cout_g = liste_fermee[n].cout_g + (((n.first - i)&&(n.second - j))?14:10);//distance(i,j,n.first,n.second);
                tmp.cout_h = //max(abs(arrivee.x - i), abs(arrivee.y - j));//*10;     //Chebyshev distance
                             (abs(arrivee.x - i) + abs(arrivee.y - j));//*10;         //Manhattan distance
                             //sqrt((arrivee.x - i)*(arrivee.x - i) + (arrivee.y - j)*(arrivee.y - j));

                tmp.cout_f = tmp.cout_g + tmp.cout_h;
                tmp.parent = n;
                //cout << "(distx==disty)?14:10 = " << ((distx==disty)?14:10) <<", liste_fermee[n].cout_g = " << liste_fermee[n].cout_g <<", tmp.cout_g = " << tmp.cout_g << " et tmp.cout_h = " << tmp.cout_h << endl;

                if (deja_present_dans_liste(it, liste_ouverte)){
                    // le noeud est d�j� pr�sent dans la liste ouverte, il faut comparer les couts
                    if (tmp.cout_f < liste_ouverte[it].cout_f){
                        // si le nouveau chemin est meilleur, on update
                        liste_ouverte[it]=tmp;
                    }

                    // le noeud courant a un moins bon chemin, on ne change rien


                }else{
                    // le noeud n'est pas pr�sent dans la liste ouverte, on l'ajoute
                    liste_ouverte[pair<int,int>(i,j)]=tmp;
                }
            }
        }
    }
}

bool deja_present_dans_liste(pair<int,int> n, l_noeud& l){
    l_noeud::iterator i = l.find(n);
    if (i==l.end())
        return false;
    else
        return true;
}

/*
    fonction qui renvoie la cl� du meilleur noeud de la liste
*/
pair<int,int> meilleur_noeud(l_noeud& l){
    float m_coutf = l.begin()->second.cout_f;
    pair<int,int> m_noeud = l.begin()->first;

    for (l_noeud::iterator i = l.begin(); i!=l.end(); i++)
        if (i->second.cout_f< m_coutf){
            m_coutf = i->second.cout_f;
            m_noeud = i->first;

        }

    return m_noeud;
}

/*
    fonction qui passe l'�l�ment p de la liste ouverte dans la ferm�e
*/
void ajouter_liste_fermee(pair<int,int>& p){
    noeud& n = liste_ouverte[p];
    liste_fermee[p]=n;

    // il faut le supprimer de la liste ouverte, ce n'est plus une solution explorable
    if (liste_ouverte.erase(p)==0)
        cerr << "n'apparait pas dans la liste ouverte, impossible � supprimer" << endl;
    return;
}

/*
    fonction qui met
        en bleu toutes les cases du chemin final
*/
void ecrire_bmp(bool cond){
    list<sf::Vector2u>::iterator j;

    cout << "chemin final de longueur " << chemin.size() << endl;

    if(!cond)
    {
        cout << "derni�re liste ouverte : " << liste_ouverte.size() << " �l�ments" << endl;
        cout << "derni�re liste fermee : " << liste_fermee.size() << " �l�ments" << endl;

        for (j=chemin.begin(); j !=chemin.end(); j++)
            s.setPixel(j->x,j->y,sf::Color::Blue);
        definir_points_clees();

        cout << "nombre de points de passage : " << passage.size() << endl;

    }


    if (!s.saveToFile("resultat.bmp"))
    {
        cerr << "erreur lors de la sauvegarde du bmp" << endl;
    }
}

void retrouver_chemin(){
    // l'arriv�e est le dernier �l�ment de la liste ferm�e.
    noeud& tmp = liste_fermee[std::pair<int, int>(arrivee.x,arrivee.y)];

    sf::Vector2u n;
    pair<int,int> prec;
    n.x = arrivee.x;
    n.y = arrivee.y;
    prec.first  = tmp.parent.first;
    prec.second = tmp.parent.second;
    chemin.push_front(n);

    while (prec != pair<int,int>(depart.parent.first,depart.parent.second/*first*/)){
        n.x = prec.first;
        n.y = prec.second;
        chemin.push_front(n);

        tmp = liste_fermee[tmp.parent];
        prec.first  = tmp.parent.first;
        prec.second = tmp.parent.second;
    }
}


void convertcartpol(const pair<int,int>& entree, pair<double,double>& point) //entr�e (x,y) retourne (r,theta)
{
    point.first = sqrt(entree.first*entree.first + entree.second*entree.second);
    if(entree.first==0)
    {
        if(entree.second==0)
        {
            point.second = 0;
        }
        else
        {
            point.second = ((entree.second)<0) ? -M_PI_2 : M_PI_2;
        }
    }
    else
    {
        point.second = (entree.first>0) ? atan(((double)entree.second)/(entree.first)) : atan(((double)entree.second)/(entree.first)) + M_PI;
    }

}

void convertpolcart(const pair<double,double>& entree, pair<int,int>& point)
{
    point.first = round(entree.first*cos(entree.second));
    point.second = round(entree.first*sin(entree.second));
}

bool tracer_droite(pair<int,int>& dep, pair<int,int>& arr)
{
    sf::Vector2u result;
    double m,p,pas,i;
    pair<double,double> deppol;
    pair<int,int> resultc;

    resultc.first = arr.first - dep.first;
    resultc.second = arr.second - dep.second;

    cout << "(x1,y1) = (" << resultc.first << ", " << resultc.second << ")" << endl;
    convertcartpol(resultc, deppol);

    int a = deppol.first;
    for(i=0;i<=a;i++)
    {
        deppol.first = i;
        convertpolcart(deppol,resultc);
        resultc.first += dep.first;
        resultc.second += dep.second;

        if(s.getPixel(resultc.first, resultc.second) == sf::Color::Black)
        {
            return false;
        }
    }
    for(i=0;i<=a+0.2;i+=1.2)
    {
        deppol.first = i;
        convertpolcart(deppol,resultc);
        resultc.first += dep.first;
        resultc.second += dep.second;

        cout << "(x,y) = (" << resultc.first << ", " << resultc.second << ")" << endl;

        s.setPixel(resultc.first, resultc.second,sf::Color::Blue);
        result.x = resultc.first;
        result.y = resultc.second;
        chemin.push_back(result);
    }
    //cout << "(x1,y1) = (" << dep.first << ", " << dep.second << ")" << endl;

    /*int a=dep.first,b=arr.first;
    if(dep.first != arr.first)
    {
        if(dep.first < arr.first)
        {
            m = (double)(arr.second - dep.second)/(arr.first - dep.first);
        }
        else
        {
            m = (double)(dep.second - arr.second)/(dep.first - arr.first);
            a = arr.first;
            b = dep.first;
        }deppol.first = arr.first - dep.first;

        pas = 1/(abs(m)+1);
        p = arr.second - m * arr.first;

        cout << "pas = " << pas << endl;

        for(i = a; (i < arrivee.x);i += pas)
        {
            if(s.getPixel(i, round(m*i+p)) == sf::Color::Black)
            {
                return false;
            }
        }

        for(i = a; (i < arrivee.x);i += pas)
        {
            cout << "(round(m*i+p) = " << round(m*i+p) << " et i = " << i << endl;
            s.setPixel(i, round(m*i+p),sf::Color::Blue);
            result.x = i;
            result.y = round(m*i+p);
            chemin.push_back(result);
        }
        chemin.unique();
    }
    else
    {
        if(dep.second < arr.second)
        {
            a = dep.second;
            b = arr.second;
        }
        else
        {
            a = arr.second;
            b = dep.second;
        }
        for(i = dep.second; i <= arr.second;i++)
        {
            if(s.getPixel(dep.first, i) == sf::Color::Black)
            {
                return false;
            }
        }
        for(i = a; i <= b;i++)
        {
            s.setPixel(dep.first,i,sf::Color::Blue);
            result.x = dep.first;
            result.y = i;
            chemin.push_back(result);
        }
    }*/
    return true;
}

void definir_points_clees()
{
    int depx, depy, i=0, ctrl=0;
    bool droit = false;
    sf::Vector2u *p;

    p=chemin.get_allocator().allocate(chemin.size());

    for (list<sf::Vector2u>::iterator j=chemin.begin(); j !=chemin.end(); j++)
    {
        p[i] = *j;

        if(ctrl > 1)
        {
            if((depx == p[i].x - p[i-1].x) && (depy == p[i].y - p[i-1].y))
            {
                droit = true;
            }
            else
            {
                droit = false;
            }

            if(!droit)
            {
                passage.push_back(p[i-1]);
                s.setPixel(p[i-1].x,p[i-1].y,sf::Color::Red);
            }
        }
        if(ctrl)
        {
            depx = p[i].x - p[i-1].x;
            depy = p[i].y - p[i-1].y;
        }

        i++;
        ctrl++;
    }

    passage.push_back(p[i-1]);
    s.setPixel(p[i-1].x, p[i-1].y,sf::Color::Red);

    chemin.get_allocator().deallocate(p,chemin.size());


}
