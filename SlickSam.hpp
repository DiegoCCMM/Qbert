//
// Created by diego on 29/4/21.
//

#include "Enemy.hpp"

#ifndef ALLEGRO5TUTORIAL_SLICKSAM_HPP
#define ALLEGRO5TUTORIAL_SLICKSAM_HPP

class SlickSam : public Enemy{

    int reachQberti = VACIO,  reachQbertj = VACIO;

public:
    SlickSam(const Piramide &piramide, const std::string &nom, int i, int j, int xRespectCube, int yRespectCube) : Enemy(
            piramide, nom, i, j, xRespectCube, yRespectCube) {
        setChangingGroundPower(true);
    }

    void randomMovement(int i, int j) override {

        //TODO RETOCAR MOVIMIENTO SI MUY REPETITIVO
        std::cout<<"entro 80"<<std::endl;

        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> dist4(1, 4);
        std::uniform_int_distribution<int> dist2(1, 2);
        std::uniform_int_distribution<int> dist3(1, 3);

        if(getI() == 6){//estamos abajo
            std::cout<<"estamos abajo"<<std::endl;
            if(getJ() == 0){
                //esquina abajo a la izquierda
                setDir(TOPRIGHT);
            }else if(getJ() == getI()){
                //esquina abajo a la derecha
                setDir(TOPLEFT);
            }else{  //evitamos caer abajo
                if(dist2(mt) == 1){
                    setDir(TOPRIGHT);
                }else{
                    setDir(TOPLEFT);
                }
            }


        }else if(getI() == getJ()){//estamos a la derecha
            std::cout<<"estamos der"<<std::endl;
            if(getJ() == 0) {
                //esquina arriba
                if(dist2(mt) == 1){
                    setDir(DOWNRIGHT);
                }else {
                    setDir(DOWNLEFT);
                }
            }else{//lateral derecho
                int a = dist4(mt);

                if(getDir() == DOWNRIGHT || getDir() == TOPLEFT){
                    setDir(DOWNLEFT);
                }else{
                    setDir(DOWNLEFT);
                }
            }
        }else if(getJ() == 0){//estamos a la izquierda
            std::cout<<"estamos izquierda"<<std::endl;
            int a = dist4(mt);
            if(getDir() == TOPRIGHT || getDir() == DOWNLEFT){
                setDir(DOWNRIGHT);
            }else{
                setDir(DOWNRIGHT);
            }
        }else{ //sin restriccion
            std::cout<<"sin restriccion"<<std::endl;
            int a = dist4(mt);
            if(a == 1){
                setDir(TOPLEFT);
            }else if(a == 2){
                setDir(DOWNLEFT);
            }else if(a == 3){
                setDir(DOWNRIGHT);
            }else{
                setDir(TOPRIGHT);
            }

        }

        assignIJ();

        Enemy::setJumping(true);
        Enemy::setSourceX(getSourceX()-16);


    }

    void drawBitmap() override {
            al_draw_bitmap_region(getDraw(), getSourceX() + (getDir() * 2 * sizePixelsX),
                                  getSourceY() + (sourceI * sizePixelsY), sizePixelsX, sizePixelsY*2,
                                  getX(), getY(), 0);
    }
};

#endif //ALLEGRO5TUTORIAL_SLICKSAM_HPP