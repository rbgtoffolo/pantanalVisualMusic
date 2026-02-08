#include "PersonTracker.h"

void PersonTracker::setup(int w, int h){
    // Inicializa a câmera
    vidGrabber.setVerbose(true);
    vidGrabber.setup(w, h);

    // Aloca memória para as imagens do OpenCV
    colorImg.allocate(w, h);
    grayImage.allocate(w, h);
    grayBg.allocate(w, h);
    grayDiff.allocate(w, h);

    bLearnBackground = true;
    threshold = 80; // Valor inicial para sensibilidade da subtração de fundo
    idCounter = 0;
    trackedPeople.clear();
}

void PersonTracker::update(){
    vidGrabber.update();
    
    if (vidGrabber.isFrameNew()){
        colorImg.setFromPixels(vidGrabber.getPixels());
        grayImage = colorImg; // Converte para escala de cinza
        
        if (bLearnBackground == true){
            grayBg = grayImage;
            bLearnBackground = false;
        }

        // Calcula a diferença absoluta entre o fundo capturado e o frame atual
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        
        // Encontra contornos (blobs)
        // Parâmetros: imagem, minArea, maxArea, nBlobs, findHoles
        // minArea = 2000 (ignora ruídos pequenos), maxArea = 1/3 da tela
        contourFinder.findContours(grayDiff, 2000, (vidGrabber.getWidth()*vidGrabber.getHeight())/3, 10, false);
        
        // --- Lógica de Rastreamento (Tracking) ---
        
        // 1. Coletar posições atuais dos blobs
        vector<ofVec2f> currentBlobs;
        for(int i = 0; i < contourFinder.nBlobs; i++){
            currentBlobs.push_back(contourFinder.blobs[i].centroid);
        }

        // 2. Preparar matching
        float maxDistance = 100.0; // Distância máxima (em pixels) para considerar a mesma pessoa
        vector<bool> blobMatched(currentBlobs.size(), false);

        // Marcar todos os rastreados como "morrendo" (dying) até provar o contrário
        for(auto & person : trackedPeople){
            person.dyingCount++;
        }

        // 3. Tentar corresponder pessoas já rastreadas com blobs novos
        for(auto & person : trackedPeople){
            int closestIndex = -1;
            float minDist = maxDistance;

            for(int i = 0; i < currentBlobs.size(); i++){
                if(blobMatched[i]) continue; // Blob já foi atribuído a outra pessoa

                float d = person.pos.distance(currentBlobs[i]);
                if(d < minDist){
                    minDist = d;
                    closestIndex = i;
                }
            }

            if(closestIndex != -1){
                // Encontrou a mesma pessoa
                person.pos = currentBlobs[closestIndex];
                person.dyingCount = 0; // Reseta contagem de desaparecimento
                blobMatched[closestIndex] = true;
            }
        }

        // 4. Remover pessoas que sumiram por muito tempo (ex: 10 frames de tolerância)
        ofRemove(trackedPeople, [](const TrackedPerson& p){
            return p.dyingCount > 10;
        });

        // 5. Adicionar novos blobs como novas pessoas
        for(int i = 0; i < currentBlobs.size(); i++){
            if(!blobMatched[i]){
                TrackedPerson newPerson;
                newPerson.id = idCounter++;
                newPerson.pos = currentBlobs[i];
                newPerson.dyingCount = 0;
                trackedPeople.push_back(newPerson);
            }
        }
    }
}

void PersonTracker::draw(float x, float y, float w, float h){
    ofSetColor(255);
    grayDiff.draw(x, y, w, h);
    contourFinder.draw(x, y, w, h);
    
    // Desenha os IDs para debug visual
    float scaleX = w / vidGrabber.getWidth();
    float scaleY = h / vidGrabber.getHeight();
    
    for(const auto& person : trackedPeople){
        if(person.dyingCount == 0){ // Só desenha se estiver visível
            ofDrawBitmapStringHighlight("ID: " + ofToString(person.id), 
                                        x + person.pos.x * scaleX, 
                                        y + person.pos.y * scaleY);
        }
    }
}

int PersonTracker::getNumPeople() const {
    return trackedPeople.size();
}

ofVec2f PersonTracker::getPersonPosition(int index) const {
    if(index >= 0 && index < trackedPeople.size()){
        return trackedPeople[index].pos;
    }
    return ofVec2f(0,0);
}

int PersonTracker::getPersonId(int index) const {
    if(index >= 0 && index < trackedPeople.size()){
        return trackedPeople[index].id;
    }
    return -1;
}

void PersonTracker::setThreshold(int value){
    threshold = value;
}

void PersonTracker::resetBackground(){
    bLearnBackground = true;
}