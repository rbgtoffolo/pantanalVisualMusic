#include "VideoManager.h"

void VideoManager::setup(int w, int h, float loopFadeDuration){
  
    width = w;
    height = h;
    colorImg.allocate(w, h);
    grayImage.allocate(w, h);
    grayBg.allocate(w, h);
    grayDiff.allocate(w, h);

    fboOriginal.allocate(w, h, GL_RGBA);
    fboInverted.allocate(w, h, GL_RGBA);
    bCvInitialized = true;


    loopCrossfadeDuration = loopFadeDuration;
    ofDirectory dir("videos");
    dir.allowExt("mp4");
    dir.allowExt("mov");
    dir.allowExt("avi");
    dir.listDir();
    dir.sort();
    
    for(int i = 0; i < dir.size(); i++){
        ofVideoPlayer player;
        player.load(dir.getPath(i));
        videos.push_back(player);
        
        ofVideoPlayer loopPlayer;
        loopPlayer.load(dir.getPath(i));
        loopPlayer.setLoopState(OF_LOOP_NONE);
        loopVideos.push_back(loopPlayer);
    }
}

void VideoManager::update(){

    // Atualiza o vídeo atual
    if(currentIndex != -1 && currentIndex < videos.size()){
        videos[currentIndex].update();
        
        // Lógica de crossfade em loop (se habilitado e não estiver trocando de vídeo)
        if(loopCrossfadeDuration > 0 && !isFading){
            float duration = videos[currentIndex].getDuration();
            float position = videos[currentIndex].getPosition() * duration;
            float timeRemaining = duration - position;
            
            if(timeRemaining <= loopCrossfadeDuration){
                if(!isLoopFading){
                    isLoopFading = true;
                    loopVideos[currentIndex].play();
                    loopVideos[currentIndex].setPosition(0);
                }
                loopVideos[currentIndex].update();
                
                if(videos[currentIndex].getIsMovieDone() || position >= duration - 0.05f){
                    videos[currentIndex].stop();
                    std::swap(videos[currentIndex], loopVideos[currentIndex]);
                    isLoopFading = false;
                }
            }
        }
    }
    
    // Se estiver em transição, atualiza o próximo vídeo e o timer
    if(isFading && nextIndex != -1 && nextIndex < videos.size()){
        videos[nextIndex].update();
        
        fadeTimer += ofGetLastFrameTime();
        if(fadeTimer >= fadeDuration){
            // Fim do fade
            isFading = false;
            videos[currentIndex].stop();
            if(isLoopFading){
                loopVideos[currentIndex].stop();
                isLoopFading = false;
            }
            currentIndex = nextIndex;
            nextIndex = -1;
        }
    }

    if (currentIndex != -1) {
        const ofPixels& pix = getFramePixels();
        if (pix.isAllocated()) {
            // Verifica se as dimensões do vídeo diferem das imagens alocadas (ROI mismatch fix)
            if (grayBg.getWidth() != pix.getWidth() || grayBg.getHeight() != pix.getHeight()) {
                int w = pix.getWidth();
                int h = pix.getHeight();
                colorImg.allocate(w, h);
                grayImage.allocate(w, h);
                grayBg.allocate(w, h);
                grayDiff.allocate(w, h);
                width = w;
                height = h;
                bLearnBackground = true;
            }

            colorImg.setFromPixels(pix);
            grayImage = colorImg;
            
            if (bLearnBackground) {
                grayBg = grayImage;
                bLearnBackground = false; 
            }

            // Calcula a diferença entre o frame atual e o fundo "estático"
            // Isso isola o movimento das ondas na margem
            grayDiff.absDiff(grayBg, grayImage);
            grayDiff.threshold(thresholdValue);
            
            // Encontra os contornos do movimento
            // Parâmetros: imagem, minArea, maxArea, nConsidered, bFindHoles
            contourFinder.findContours(grayDiff, 20, (width * height) / 4, 10, false);

            // Converte para polilinhas suavizadas para o grafismo
            waveLines.clear();
            
            // Calcula a escala para adaptar as linhas da resolução do vídeo para a resolução da janela/tela
            float scaleX = (float)ofGetWidth() / grayDiff.getWidth();
            float scaleY = (float)ofGetHeight() / grayDiff.getHeight();

            for (int i = 0; i < contourFinder.nBlobs; i++) {
                ofPolyline p;
                for (int j = 0; j < contourFinder.blobs[i].pts.size(); j++) {
                    p.addVertex(contourFinder.blobs[i].pts[j].x, contourFinder.blobs[i].pts[j].y);
                }
                p.close();
                // Escala a polilinha para coincidir com o tamanho da tela
                p.scale(scaleX, scaleY);
                // Suavização para evitar o aspecto "pixelado" do OpenCV
                waveLines.push_back(p.getResampledBySpacing(5).getSmoothed(3));
            }
        }
    }
}

void VideoManager::draw(float x, float y, float w, float h, float alpha){
    if(currentIndex == -1 || currentIndex >= videos.size()) return;

    auto drawCurrentVideoState = [&](){
        if (isFading && nextIndex != -1) {
            float pct = ofClamp(fadeTimer / fadeDuration, 0.0, 1.0);
            
            // Desenha o atual sumindo
            ofSetColor(255, 255 * (1.0 - pct));
            videos[currentIndex].draw(x, y, w, h);

            // Desenha o próximo aparecendo
            ofSetColor(255, 255 * pct);
            videos[nextIndex].draw(x, y, w, h);
        } else if (isLoopFading) {
            float duration = videos[currentIndex].getDuration();
            float position = videos[currentIndex].getPosition() * duration;
            float timeRemaining = duration - position;
            float pct = 1.0f - ofClamp(timeRemaining / loopCrossfadeDuration, 0.0f, 1.0f);

            ofSetColor(255, 255 * (1.0 - pct));
            videos[currentIndex].draw(x, y, w, h);

            ofSetColor(255, 255 * pct);
            loopVideos[currentIndex].draw(x, y, w, h);
        } else {
            ofSetColor(255);
            videos[currentIndex].draw(x, y, w, h);
        }
    };

    fboOriginal.begin();
    ofClear(0, 0, 0, 0); // Limpa o FBO

    ofPushStyle();
    // Para o crossfade entre vídeos (isFading), usamos o modo aditivo (ADD)
    // para evitar o "dip" de brilho que acontece com o alpha blending normal.
    if (isFading) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
    } else {
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    }
    drawCurrentVideoState();
    ofPopStyle();
    fboOriginal.end();

    // --- NOVO: Etapa de Blur Gaussiano ---
   
    if (blurAmount > 0.5f) { 
        ofPixels tempPixels;
        fboOriginal.readToPixels(tempPixels); 

        if (tempPixels.isAllocated()) {
           
            static ofPixels rgbPixels; // static para evitar realocação a cada frame
            if (!rgbPixels.isAllocated() || rgbPixels.getWidth() != tempPixels.getWidth() || rgbPixels.getHeight() != tempPixels.getHeight()) {
                rgbPixels.allocate(tempPixels.getWidth(), tempPixels.getHeight(), OF_PIXELS_RGB);
            }

            const unsigned char* src = tempPixels.getData();
            unsigned char* dst = rgbPixels.getData();
            size_t totalPixels = tempPixels.getWidth() * tempPixels.getHeight();
            for (size_t i = 0; i < totalPixels; i++) {
                dst[i * 3]     = src[i * 4];     // R
                dst[i * 3 + 1] = src[i * 4 + 1]; // G
                dst[i * 3 + 2] = src[i * 4 + 2]; // B
            }

            blurImg.setFromPixels(rgbPixels);

            int oddBlurAmount = static_cast<int>(blurAmount);
            if (oddBlurAmount < 1) oddBlurAmount = 1;
            if (oddBlurAmount % 2 == 0) {
                oddBlurAmount++;
            }
            blurImg.blurGaussian(oddBlurAmount);

            fboOriginal.begin();
            ofClear(0, 0, 0, 0);
            ofPushStyle();
            ofSetColor(255);
            blurImg.draw(0, 0);
            ofPopStyle();
            fboOriginal.end();
        }
    }

    // Se houver efeito de inversão, o preparamos e o mesclamos de volta no FBO original
    if (invertAmount > 0.0f) {
        fboInverted.begin();
        ofClear(0, 0, 0, 0);
        ofPushStyle();

        ofSetColor(255);
        ofDrawRectangle(0, 0, fboInverted.getWidth(), fboInverted.getHeight());

        glEnable(GL_BLEND);
        glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);

        fboOriginal.draw(0, 0);
        ofPopStyle();
        fboInverted.end();

        // Agora, mescla o efeito de inversão de volta no FBO original
        fboOriginal.begin();
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        ofSetColor(255, 255 * invertAmount);
        fboInverted.draw(0, 0);
        ofDisableBlendMode();
        ofPopStyle();
        fboOriginal.end();
    }

    // Finalmente, desenha o FBO original, que agora contém todos os efeitos (vídeo, blur, inversão).
    // Usamos o valor de 'alpha' passado para a função para controlar a opacidade final.
    ofPushStyle();
    ofEnableAlphaBlending();
    ofSetColor(255, 255, 255, 255 * alpha);
    fboOriginal.draw(x, y, w, h);
    ofPopStyle();
}

void VideoManager::onWindowResized(int w, int h) {
    fboOriginal.allocate(w, h, GL_RGBA);
    fboInverted.allocate(w, h, GL_RGBA);

    if (blurImg.bAllocated) {
        blurImg.clear();
    }
}

void VideoManager::changeVideo(int index, float fadeTime){
    if(index < 0 || index >= videos.size()) return;
    if(index == currentIndex && !isFading) return;
    if(isFading) return;

    if(isLoopFading){
        videos[currentIndex].stop();
        std::swap(videos[currentIndex], loopVideos[currentIndex]);
        isLoopFading = false;
    }

    // Se já estiver tocando algo, prepara o fade
    if(currentIndex != -1){
        
        nextIndex = index;
        videos[nextIndex].play();
        
        if(loopCrossfadeDuration > 0) videos[nextIndex].setLoopState(OF_LOOP_NONE);
        else videos[nextIndex].setLoopState(OF_LOOP_NORMAL);
        
        fadeDuration = fadeTime;
        fadeTimer = 0.0f;
        isFading = true;
    } else {
        // Primeiro vídeo a tocar (sem fade)
        currentIndex = index;
        videos[currentIndex].play();
        
        if(loopCrossfadeDuration > 0) videos[currentIndex].setLoopState(OF_LOOP_NONE);
        else videos[currentIndex].setLoopState(OF_LOOP_NORMAL);
        
        isFading = false;
    }

    bLearnBackground = true; 
    waveLines.clear();
}

int VideoManager::getVideoCount() const {
    return videos.size();
}

void VideoManager::setBlurAmount(float amount) {
    blurAmount = ofClamp(amount, 0.0f, 100.0f); 
}

void VideoManager::updateInvert(float amplitude, float startThreshold, float fullThreshold) {
    invertAmount = ofMap(amplitude, startThreshold, fullThreshold, 0.0, 1.0, true);
}

void VideoManager::setSpeed(float speed) {
    if (currentIndex != -1 && currentIndex < videos.size()) {
        videos[currentIndex].setSpeed(speed);
        if (currentIndex < loopVideos.size()) {
            loopVideos[currentIndex].setSpeed(speed);
        }
    }
    if (isFading && nextIndex != -1 && nextIndex < videos.size()) {
        videos[nextIndex].setSpeed(speed);
    }
}

const ofPixels& VideoManager::getFramePixels() {
    if (currentIndex < 0 || currentIndex >= videos.size()) {
        static ofPixels empty;
        return empty;
    }

    // Verifica se estamos em transição (Crossfade entre vídeos)
    bool fading = (isFading && nextIndex != -1 && nextIndex < videos.size());
    // Verifica se estamos em transição de Loop
    bool loopFading = (isLoopFading && loopCrossfadeDuration > 0);

    if (fading || loopFading) {
        ofVideoPlayer* p1 = &videos[currentIndex];
        ofVideoPlayer* p2 = nullptr;
        float pct = 0.0f;

        if (fading) {
            p2 = &videos[nextIndex];
            pct = ofClamp(fadeTimer / fadeDuration, 0.0f, 1.0f);
        } else {
            p2 = &loopVideos[currentIndex];
            float duration = p1->getDuration();
            float position = p1->getPosition() * duration;
            float timeRemaining = duration - position;
            pct = 1.0f - ofClamp(timeRemaining / loopCrossfadeDuration, 0.0f, 1.0f);
        }

        if (p1->isLoaded() && p2->isLoaded()) {
            if (!blendedPixels.isAllocated() || blendedPixels.getWidth() != p1->getWidth() || blendedPixels.getHeight() != p1->getHeight()) {
                blendedPixels.allocate(p1->getWidth(), p1->getHeight(), OF_PIXELS_RGB);
            }

            if (p1->getWidth() == p2->getWidth() && p1->getHeight() == p2->getHeight()) {
                const ofPixels& pix1 = p1->getPixels();
                const ofPixels& pix2 = p2->getPixels();
                
                const unsigned char* s1 = pix1.getData();
                const unsigned char* s2 = pix2.getData();
                unsigned char* dst = blendedPixels.getData();
                size_t total = pix1.size();
                
                int alpha = (int)(pct * 255);
                int invAlpha = 255 - alpha;
                
                for(size_t i=0; i<total; i++){
                    dst[i] = (unsigned char)((s1[i] * invAlpha + s2[i] * alpha) / 255);
                }
                return blendedPixels;
            }
        }
    }

    return videos[currentIndex].getPixels();
}