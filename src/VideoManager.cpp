#include "VideoManager.h"

void VideoManager::setup(float loopFadeDuration){
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
        
        // Carrega uma segunda instância para o crossfade de loop
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
}

void VideoManager::draw(float x, float y, float w, float h){
    if(currentIndex != -1 && currentIndex < videos.size()){
        ofSetColor(255);
        if(isFading && nextIndex != -1){
            float pct = ofClamp(fadeTimer / fadeDuration, 0.0, 1.0);
            
            // Desenha o atual sumindo
            ofSetColor(255, 255 * (1.0 - pct));
            videos[currentIndex].draw(x, y, w, h);
            
            // Desenha o próximo aparecendo
            ofSetColor(255, 255 * pct);
            videos[nextIndex].draw(x, y, w, h);
        } else if(isLoopFading) {
            float duration = videos[currentIndex].getDuration();
            float position = videos[currentIndex].getPosition() * duration;
            float timeRemaining = duration - position;
            float pct = 1.0f - ofClamp(timeRemaining / loopCrossfadeDuration, 0.0f, 1.0f);
            
            ofSetColor(255, 255 * (1.0 - pct));
            videos[currentIndex].draw(x, y, w, h);
            
            ofSetColor(255, 255 * pct);
            loopVideos[currentIndex].draw(x, y, w, h);
        } else {
            videos[currentIndex].draw(x, y, w, h);
        }
        ofSetColor(255); // Restaura cor
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
}

int VideoManager::getVideoCount() const {
    return videos.size();
}