#include "ColorAnalyzer.h"

vector<ColorProfile> ColorAnalyzer::getDominantColors(const ofPixels& pixels, int k) {
    vector<ColorProfile> results;

    if (!pixels.isAllocated() || k <= 0) return results;

    // 1. Usar ofxCvColorImage para manipulação (Redimensionar)
    // Isso utiliza a infraestrutura do ofxOpenCv como solicitado
    ofxCvColorImage cvImg;
    cvImg.setUseTexture(false); // Otimização: não carregar textura na GPU
    cvImg.allocate(pixels.getWidth(), pixels.getHeight());
    cvImg.setFromPixels(pixels);
    
    // Redimensionar para 100x100 para acelerar o K-Means
    cvImg.resize(100, 100);
    
    // 2. Converter para cv::Mat e preparar dados para K-Means
    // Obtemos o IplImage* e convertemos para cv::Mat, depois para float e reshape
    cv::Mat imgMat = cv::cvarrToMat(cvImg.getCvImage());
    cv::Mat samples;
    imgMat.convertTo(samples, CV_32F);
    samples = samples.reshape(1, samples.total()); // (N pixels, 3 canais)

    // 3. Executar K-Means Clustering
    cv::Mat labels;
    cv::Mat centers;
    int attempts = 3;
    // TermCriteria: Para após 10 iterações ou quando a mudança for menor que 1.0 (epsilon)
    cv::kmeans(samples, k, labels, 
               cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0), 
               attempts, cv::KMEANS_PP_CENTERS, centers);

    // 4. Contar ocorrências de cada cluster (label) para calcular dominância
    std::map<int, int> counts;
    for (int i = 0; i < labels.rows; i++) {
        counts[labels.at<int>(i)]++;
    }

    // 5. Montar o resultado
    int numPixels = samples.rows;
    for (int i = 0; i < k; i++) {
        ColorProfile profile;

        // Centros retornam float, convertemos para ofColor
        float r = centers.at<float>(i, 0);
        float g = centers.at<float>(i, 1);
        float b = centers.at<float>(i, 2);

        profile.color.set(r, g, b);
        profile.hue = profile.color.getHue();
        profile.saturation = profile.color.getSaturation();
        profile.brightness = profile.color.getBrightness();
        profile.percentage = (float)counts[i] / (float)numPixels;

        results.push_back(profile);
    }

    // 6. Ordenar por dominância (maior para menor)
    std::sort(results.begin(), results.end(), [](const ColorProfile& a, const ColorProfile& b) {
        return a.percentage > b.percentage;
    });

    return results;
}
