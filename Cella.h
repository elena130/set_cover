//
// Created by Elena on 27/03/2025.
//

#ifndef CELLA_H
#define CELLA_H



class Cella {
public:
    int value;
    Cella* up;
    Cella* right;
    Cella* down;
    Cella* left;

    Cella(int v) {
        value = v;
    }

    Cella() {
        value = 0;
    }
};



#endif //CELLA_H
