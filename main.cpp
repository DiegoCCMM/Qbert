#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_primitives.h>
#include <list>
#include <fstream>
#include <iostream>

#include "Piramide.hpp"
#include "Enemy.hpp"
#include "Coily.hpp"
#include "PantallaInicial.hpp"
#include "PantallaInfoNivel.hpp"
#include "Escena.hpp"
#include "Creditos.hpp"

#define NUMERODESAMPLES 10

float scale;
float WIDTH, HEIGHT;

void must_init(bool test, const char *description);
void guardarResize();

int main() {
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");

    // Memoria a la hora del reescalado
    std::ifstream file("../pantalla.txt");
    file >> WIDTH >> HEIGHT >> scale;
    file.close();

    ALLEGRO_TRANSFORM camera;

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 30.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    must_init(queue, "queue");

    al_set_new_display_flags(ALLEGRO_RESIZABLE);
    ALLEGRO_DISPLAY *disp = al_create_display(WIDTH, HEIGHT);
    al_set_window_title(disp, "Q*Bert");
    must_init(disp, "display");

    ALLEGRO_FONT *font = al_create_builtin_font();
    must_init(font, "font");

    ALLEGRO_KEYBOARD_STATE keyState;
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    must_init(al_init_image_addon(), "image addon");

    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(NUMERODESAMPLES);

    al_identity_transform(&camera);

    al_acknowledge_resize(disp);

    al_resize_display(disp, WIDTH, HEIGHT);

    al_scale_transform(&camera, scale, scale);
    al_use_transform(&camera);

    Creditos credit = Creditos();
    Escena escena = Escena(WIDTH/scale, HEIGHT/scale);
    bool redraw = true;
    ALLEGRO_EVENT event;
    al_start_timer(timer);

    /*************************
     *       GAME LOOP       *
     *************************/

    int cuboID = 0, coilyID = 0, slicksamID = 0, blobID = 0, level = 1;
    // UP, DOWN, LEFT, RIGHT (inicialmente)
    // ARRIBADER, ABAJOIZQ, ARRIBAIZQ, ABAJODER
    int controls[4] = {84, 85, 82, 83};
    bool backdoor;

    // Pantallas/Estados:
    // INICIO, INFONIVEL, JUEGO, CREDITOS(registro nombre y highscore), CLOSE

    inicioIntro:
    {
        // Pantalla inicial e instrucciones
        PantallaInicial init = PantallaInicial(WIDTH/scale, HEIGHT/scale,
                                cuboID, coilyID, slicksamID, blobID, controls, level);
        inicio:
        {
//            goto juegoIntro; // Descomentar para ir directamente al juego
            //goto creditosIntro; // Descomentar ir directamente a los creditos
            al_wait_for_event(queue, &event);
            al_get_keyboard_state(&keyState);

            switch (event.type) {
                case ALLEGRO_EVENT_TIMER:

                    redraw = true;
                    init.movement();
                    break;

                case ALLEGRO_EVENT_KEY_DOWN:

                    switch (event.keyboard.keycode) {
                        case ALLEGRO_KEY_ENTER:
                            if(init.pant == 0){ // INICIO
                                init.destroy();

                                level = init.level;
                                cuboID = init.cuboID, coilyID = init.coilyID,
                                slicksamID = init.slicksamID, blobID = init.blobID;
                                // UP, DOWN, LEFT, RIGHT
                                controls[0] = init.controls[0], controls[1] = init.controls[1],
                                controls[2] = init.controls[2], controls[3] = init.controls[3];

                                escena.setLevel(level);
                                backdoor = level != 1;

                                goto infonivelIntro;

                            } else if(init.pant == 1) { // Instrucciones
                                init.escenarioInit();
                            }
                            else if(init.pant == 2 || init.pant == 3){ // Menu/controles
                                init.accionMenu(event.keyboard.keycode);
                            }

                            break;

                        case ALLEGRO_KEY_F1:
                            init.escenarioMenu();
                            break;

                        default:
                            init.accionMenu(event.keyboard.keycode);
                    }

                    break;

                case ALLEGRO_EVENT_DISPLAY_RESIZE:

                    al_identity_transform(&camera);

                    // datos previos al resize
                    WIDTH = al_get_display_width(disp);
                    HEIGHT = al_get_display_height(disp);

                    al_acknowledge_resize(disp);

                    if(HEIGHT != (float)al_get_display_height(disp)){
                        WIDTH = (330.0/280.0)*(float)al_get_display_height(disp);
                        al_resize_display(disp, WIDTH, (float)al_get_display_height(disp));
                        scale += ((float) al_get_display_height(disp) - (float) HEIGHT) * 0.0035f;
                    } else if(WIDTH != (float)al_get_display_width(disp)){
                        HEIGHT = (float)al_get_display_width(disp)/(330.0/280.0);
                        al_resize_display(disp, (float)al_get_display_width(disp), HEIGHT);
                        scale += ((float) al_get_display_width(disp) - (float) WIDTH) * 0.0035f;
                    }

                    al_scale_transform(&camera, scale, scale);
                    al_use_transform(&camera);

                    WIDTH = al_get_display_width(disp);
                    HEIGHT = al_get_display_height(disp);

                    // RESIZE ALL ITEMS
                    init.resize(WIDTH / scale, HEIGHT / scale);

                    guardarResize();

                    break;

                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    init.destroy();
                    goto close;

            }
            if (redraw && al_is_event_queue_empty(queue)) {
                //REDRAW THE IMAGE WITH EVERYTHING
                al_clear_to_color(al_map_rgb(49, 33, 121));

                init.drawBitmap();

                al_flip_display();
                redraw = false;
            }

            goto inicio;
        };
    };

    // PREVIO AL NIVEL
    infonivelIntro:
    {
        if(escena.getLevel()>9) goto creditos;

        PantallaInfoNivel infonivel = PantallaInfoNivel(WIDTH/scale, HEIGHT/scale);
        if(escena.getLevel() == 1) infonivel.sonidoMoneda();
        infonivel.startLevel(escena.getLevel());

        infonivel:
        {
            al_wait_for_event(queue, &event);
            al_get_keyboard_state(&keyState);

            switch (event.type) {
                case ALLEGRO_EVENT_TIMER:

                    redraw = true;
                    infonivel.movement();

                    if (infonivel.fin()) {
                        goto juegoIntro;
                    }

                    break;

                case ALLEGRO_EVENT_DISPLAY_RESIZE:

                    al_identity_transform(&camera);

                    // datos previos al resize
                    WIDTH = al_get_display_width(disp);
                    HEIGHT = al_get_display_height(disp);

                    al_acknowledge_resize(disp);

                    if(HEIGHT != (float)al_get_display_height(disp)){
                        WIDTH = (330.0/280.0)*(float)al_get_display_height(disp);
                        al_resize_display(disp, WIDTH, (float)al_get_display_height(disp));
                        scale += ((float) al_get_display_height(disp) - (float) HEIGHT) * 0.0035f;
                    } else if(WIDTH != (float)al_get_display_width(disp)){
                        HEIGHT = (float)al_get_display_width(disp)/(330.0/280.0);
                        al_resize_display(disp, (float)al_get_display_width(disp), HEIGHT);
                        scale += ((float) al_get_display_width(disp) - (float) WIDTH) * 0.0035f;
                    }

                    al_scale_transform(&camera, scale, scale);
                    al_use_transform(&camera);

                    WIDTH = al_get_display_width(disp);
                    HEIGHT = al_get_display_height(disp);

                    // RESIZE ALL ITEMS
                    infonivel.resize(WIDTH / scale, HEIGHT / scale);

                    guardarResize();

                    break;

                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    goto close;
                    break;

            }
            if (redraw && al_is_event_queue_empty(queue)) {
                //REDRAW THE IMAGE WITH EVERYTHING
                al_clear_to_color(al_map_rgb(0, 0, 0));

                infonivel.drawBitmap();

                al_flip_display();
                redraw = false;
            }
        };

        goto infonivel;
    };

    // GAME LOOP
    juegoIntro:
    {
        escena.load(WIDTH/scale, HEIGHT/scale, backdoor, cuboID, coilyID, slicksamID, blobID);
        bool pause = false;

        juego:
        {
            if(escena.piramideCompleta()) {
                escena.setRound(escena.getRound()+1);
                if(escena.getRound() > 4){
                    escena.setRound(1);
                    escena.setLevel(escena.getLevel()+1);

                    if(escena.getLevel() > 9){
                        goto creditosIntro;
                    }

                    goto infonivelIntro;
                }

                goto juegoIntro;
            }
            else if(escena.isGameover()){
                goto creditosIntro;
            }

            al_wait_for_event(queue, &event);
            al_get_keyboard_state(&keyState);

            switch (event.type) {
                case ALLEGRO_EVENT_TIMER:

                    redraw = true;
                    if(!pause) {
                        escena.movementAll();
                        escena.generarEnemigos();
                    }

                    break;

                case ALLEGRO_EVENT_KEY_DOWN:

                    if (!escena.qbert.isJumping() && !pause) {
                        if(event.keyboard.keycode == controls[2]){
                            escena.setMoveQBert(TOPLEFT);
                        }
                        else if(event.keyboard.keycode == controls[3]) {
                            escena.setMoveQBert(DOWNRIGHT);
                        }
                        else if(event.keyboard.keycode == controls[0]) {
                            escena.setMoveQBert(TOPRIGHT);
                        }
                        else if(event.keyboard.keycode == controls[1]) {
                            escena.setMoveQBert(DOWNLEFT);
                        }
                        else if(event.keyboard.keycode == ALLEGRO_KEY_PAD_PLUS ||
                                event.keyboard.keycode == ALLEGRO_KEY_EQUALS) {
                            escena.qbert.setLives(escena.qbert.getLives() + 1);
                        }
                        else if(event.keyboard.keycode == ALLEGRO_KEY_ENTER){
                            escena.setPiramideCompleta();
                        }
                    }
                    else if(pause){
                        switch (event.keyboard.keycode) {
                            case ALLEGRO_KEY_LEFT:
                            case ALLEGRO_KEY_RIGHT:
                                escena.posPause = ++escena.posPause%2;
                                break;
                            case ALLEGRO_KEY_ENTER:
                                if(escena.posPause == 0) goto inicioIntro;
                                else goto close;
                                break;
                        }
                    }

                    if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE){ // ESC
                        pause = !pause;
                    }

                    break;

                case ALLEGRO_EVENT_DISPLAY_RESIZE:

                    al_identity_transform(&camera);

                    // datos previos al resize
                    WIDTH = al_get_display_width(disp);
                    HEIGHT = al_get_display_height(disp);

                    al_acknowledge_resize(disp);

                    if(HEIGHT != (float)al_get_display_height(disp)){
                        WIDTH = (330.0/280.0)*(float)al_get_display_height(disp);
                        al_resize_display(disp, WIDTH, (float)al_get_display_height(disp));
                        scale += ((float) al_get_display_height(disp) - (float) HEIGHT) * 0.0035f;
                    } else if(WIDTH != (float)al_get_display_width(disp)){
                        HEIGHT = (float)al_get_display_width(disp)/(330.0/280.0);
                        al_resize_display(disp, (float)al_get_display_width(disp), HEIGHT);
                        scale += ((float) al_get_display_width(disp) - (float) WIDTH) * 0.0035f;
                    }

                    al_scale_transform(&camera, scale, scale);
                    al_use_transform(&camera);

                    WIDTH = al_get_display_width(disp);
                    HEIGHT = al_get_display_height(disp);

                    // RESIZE ALL ITEMS
                    escena.resizAll(WIDTH/scale, HEIGHT/scale);

                    guardarResize();

                    break;


                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    goto close;
                    break;

            }
            if (redraw && al_is_event_queue_empty(queue)) {
                //REDRAW THE IMAGE WITH EVERYTHING
                al_clear_to_color(al_map_rgb(0, 0, 0));

                escena.drawAll();
                if(pause) escena.drawPause(); // TODO

                al_flip_display();
                redraw = false;
            }

            goto juego;
        };
    };

    // CREDITOS
    creditosIntro:
    {   escena.setLevel(1);
        escena.setRound(1);
        credit.loadPantalla(escena.getPuntuacion(), WIDTH/scale, HEIGHT/scale);
        creditos:
        {
            if(credit.isFinish()) goto inicioIntro;

            al_wait_for_event(queue, &event);
            al_get_keyboard_state(&keyState);

            switch (event.type) {
                case ALLEGRO_EVENT_TIMER:

                    redraw = true;
                    credit.movement();
                    break;

                case ALLEGRO_EVENT_KEY_DOWN:

                    //std::cout << event.keyboard.keycode << std::endl;
                    credit.write(event.keyboard.keycode);

                    break;

                case ALLEGRO_EVENT_DISPLAY_RESIZE:

                    al_identity_transform(&camera);

                    // datos previos al resize
                    WIDTH = al_get_display_width(disp);
                    HEIGHT = al_get_display_height(disp);

                    al_acknowledge_resize(disp);

                    if(HEIGHT != (float)al_get_display_height(disp)){
                        WIDTH = (330.0/280.0)*(float)al_get_display_height(disp);
                        al_resize_display(disp, WIDTH, (float)al_get_display_height(disp));
                        scale += ((float) al_get_display_height(disp) - (float) HEIGHT) * 0.003f;
                    } else if(WIDTH != (float)al_get_display_width(disp)){
                        HEIGHT = (float)al_get_display_width(disp)/(330.0/280.0);
                        al_resize_display(disp, (float)al_get_display_width(disp), HEIGHT);
                        scale += ((float) al_get_display_width(disp) - (float) WIDTH) * 0.003f;
                    }

                    al_scale_transform(&camera, scale, scale);
                    al_use_transform(&camera);

                    WIDTH = al_get_display_width(disp);
                    HEIGHT = al_get_display_height(disp);

                    // RESIZE ALL ITEMS
                    credit.resize(WIDTH / scale, HEIGHT / scale);

                    guardarResize();

                    break;

                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    goto close;

            }
            if (redraw && al_is_event_queue_empty(queue)) {
                //REDRAW THE IMAGE WITH EVERYTHING
                al_clear_to_color(al_map_rgb(0, 0, 0));

                credit.drawBitmap();

                al_flip_display();
                redraw = false;
            }

            goto creditos;
        };
    };

    close:
    /*************************
     *     END GAME LOOP     *
     *************************/

    // DESTRUIR OBJETOS ESCENA
    escena.destroyAll();
    credit.destroy();

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}

void must_init(bool test, const char *description) {
    if (test) return;

    printf("No se ha podido inicializar: %s\n", description);
    exit(1);
}

void guardarResize(){
    // Guardar en memoria a la hora del reescalado
    std::ofstream file("../pantalla.txt");
    file << WIDTH << " " << HEIGHT << " " << scale;
    file.close();
}