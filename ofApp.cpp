#include "ofApp.h"


void ofApp::setup(){
    ofSetFrameRate(60);
    
    int newWidth = 1280;  // Nuova larghezza
    int newHeight = 720;  // Nuova altezza
    
    cam.setup(newWidth, newHeight);

    // Ridimensiona le immagini allocate
    colorImg.allocate(newWidth, newHeight);
    grayImage.allocate(newWidth, newHeight);
    prevGrayImage.allocate(newWidth, newHeight);
    diffImage.allocate(newWidth, newHeight);

    // Configura la dimensione della finestra
    ofSetWindowShape(newWidth, newHeight);

    // Setup GUI
    this->guiWindow->makeCurrent();
    gui.setup();
    gui.add(rectThicknessSlider.setup("Spessore", 2, 1, 10));
    gui.add(pointsSlider.setup("Punti", 10, 1, 100));
    gui.add(thresholdSlider.setup("Soglia di movimento", 30, 1, 255));
    gui.add(soundThresholdSlider.setup("Soglia suono", -20, -60, 0));
    
    // Pulsante per attivare/disattivare bianco e nero
    gui.add(blackAndWhiteButton.setup("B/N"));
    blackAndWhiteButton.addListener(this, &ofApp::blackAndWhiteButtonPressed);

    // Stato iniziale: modalità colore
    isBlackAndWhite = false;

    // slider per interlinea
    gui.add(interlineaSlider.setup("Interlinea", 5, 1, 10));

    // Aggiunta pulsante Neon e slider per l'effetto
    gui.add(neonButton.setup("Neon"));
    neonButton.addListener(this, &ofApp::neonButtonPressed);
    gui.add(glowTransparencySlider.setup("Trasparenza bagliore", 150, 0, 255));
    gui.add(glowThicknessSlider.setup("Spessore bagliore", 5, 1, 20));

    // Pulsanti per modalità Sfera e Sfondo
    gui.add(sphereButton.setup("Sfera"));
    sphereButton.addListener(this, &ofApp::sphereButtonPressed);
    gui.add(backgroundButton.setup("Sfondo"));
    backgroundButton.addListener(this, &ofApp::backgroundButtonPressed);

    gui.add(foxShadowButton.setup("Ombra della Volpe"));
    foxShadowButton.addListener(this, &ofApp::foxShadowButtonPressed);

    // Background color selection
    gui.add(backgroundLabel.setup("Sfondo", ""));
    gui.add(whiteBgButton.setup("Bianco"));
    gui.add(grayBgButton.setup("Grigio"));
    gui.add(blackBgButton.setup("Nero"));

    // Background color button listeners
    whiteBgButton.addListener(this, &ofApp::setBackgroundWhite);
    grayBgButton.addListener(this, &ofApp::setBackgroundGray);
    blackBgButton.addListener(this, &ofApp::setBackgroundBlack);

    // Movement color selection
    gui.add(movementLabel.setup("Mov", ""));
    gui.add(redButton.setup("Rosso"));
    gui.add(yellowButton.setup("Giallo"));
    gui.add(purpleButton.setup("Viola"));

    // Movement color button listeners
    redButton.addListener(this, &ofApp::setColorRed);
    yellowButton.addListener(this, &ofApp::setColorYellow);
    purpleButton.addListener(this, &ofApp::setColorPurple);

    // Filter color selection
    gui.add(filterLabel.setup("Filtro colore", ""));
    gui.add(filterColorSlider.setup("Colore da filtrare", ofColor(255, 255, 255), ofColor(0, 0), ofColor(255)));

    // Default settings
    isFoxShadowMode = false;
    isNeonEnabled = false;  // Disabilita neon all'avvio
    isSphereMode = false;   // Modalità sfera disattivata all'avvio
    drawColor = ofColor(255, 0, 0);
    backgroundColor = ofColor(255);
    filterColor = ofColor(255, 255, 255);

    originalDrawColor = drawColor;
    originalBackgroundColor = backgroundColor;

    // Setup sound stream
    smoothedVol = 0.0;
    scaledVol = 0.0;

    ofSoundStreamSettings settings;
    settings.setInListener(this);
    settings.sampleRate = 44100;
    settings.numInputChannels = 1;
    settings.bufferSize = 256;

    soundStream.setup(settings);
    soundStream.start();

    minDistance = 0;

    this->guiWindow->setVerticalSync(true);
    this->canvaWindow->setVerticalSync(true);

    ofAddListener(this->guiWindow->events().draw, this, &ofApp::drawGUI);
    ofAddListener(this->canvaWindow->events().draw, this, &ofApp::draw);
}

//--------------------------------------------------------------
void ofApp::sphereButtonPressed() {
    isSphereMode = true;
}

//--------------------------------------------------------------
void ofApp::backgroundButtonPressed() {
    isSphereMode = false;
}

//--------------------------------------------------------------
void ofApp::update(){
    cam.update();

    if (cam.isFrameNew()) {
        colorImg.setFromPixels(cam.getPixels());
        colorImg.mirror(false, true);

        grayImage = colorImg;
        diffImage.absDiff(prevGrayImage, grayImage);
        diffImage.threshold(thresholdSlider);

        // Otteniamo il colore selezionato dallo slider della GUI
        filterColor = filterColorSlider;

        ofPixels & colorPixels = colorImg.getPixels();
        ofPixels & diffPixels = diffImage.getPixels();

        // Filtra i pixel basati sul colore selezionato
        for (int y = 0; y < colorImg.getHeight(); y++) {
            for (int x = 0; x < colorImg.getWidth(); x++) {
                ofColor currentColor = colorPixels.getColor(x, y);

                // Controlla se il colore corrente è simile al colore selezionato
                if (abs(currentColor.r - filterColor.r) < 10 &&
                    abs(currentColor.g - filterColor.g) < 10 &&
                    abs(currentColor.b - filterColor.b) < 10) {
                    diffPixels.setColor(x, y, ofColor(0));  // Ignora questo pixel
                }
            }
        }

        diffImage.setFromPixels(diffPixels);
        contourFinder.findContours(diffImage, 20, (640 * 480) / 3, 10, false);
        prevGrayImage = grayImage;

        movingPixelCount = 0;
        ofPixels & pixels = diffImage.getPixels();
        for (int y = 0; y < diffImage.getHeight(); y++) {
            for (int x = 0; x < diffImage.getWidth(); x++) {
                if (pixels[y * diffImage.getWidth() + x] > 0) {
                    movingPixelCount++;
                }
            }
        }

        randomPoints.clear();
        for (int i = 0; i < pointsSlider; i++) {
            int randomIndex = ofRandom(0, contourFinder.nBlobs);
            if (randomIndex < contourFinder.nBlobs) {
                auto& blob = contourFinder.blobs[randomIndex];
                float randX = ofRandom(blob.boundingRect.x, blob.boundingRect.x + blob.boundingRect.width);
                float randY = ofRandom(blob.boundingRect.y, blob.boundingRect.y + blob.boundingRect.height);
                randomPoints.push_back(ofPoint(randX, randY));
            }
        }

        if (randomPoints.size() > 1) {
            minDistance = FLT_MAX;
            for (int i = 1; i < randomPoints.size(); i++) {
                float distance = randomPoints[i].distance(randomPoints[i - 1]);
                if (distance < minDistance) {
                    minDistance = distance;
                }
            }
        } else {
            minDistance = 0;
        }
    }
}


//--------------------------------------------------------------
void ofApp::draw(ofEventArgs& e){
    this->canvaWindow->makeCurrent();

    int newWidth = ofGetWidth();  // Nuova larghezza
    int newHeight = ofGetHeight();  // Nuova altezza

    ofPushMatrix();

    // Configura la dimensione della finestra
    ofSetWindowShape(newWidth, newHeight);

    ofPopMatrix();

    if (!isFoxShadowMode) {
            if (isBlackAndWhite) {
                // Disegna l'immagine in bianco e nero
                ofSetColor(255);
                grayImage.draw(0, 0, newWidth, newHeight);
            } else {
                // Disegna l'immagine a colori
                ofSetColor(255);
                colorImg.draw(0, 0, newWidth, newHeight);
            }
        } else {
            ofBackground(backgroundColor);
            prevGrayImage.draw(0, 0, newWidth, newHeight);
            diffImage.draw(0, 0, newWidth, newHeight);
        }

    //prevGrayImage.resize(newWidth, newHeight);
    //diffImage.resize(newWidth, newHeight);



    ofPixels & pixels = diffImage.getPixels();
    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            if (pixels[y * newWidth + x] > 0) {
                ofSetColor(drawColor, 100);
                ofDrawRectangle(x - 2, y - 2, 4, 4);
                ofSetColor(drawColor);
                ofDrawRectangle(x, y, 1, 1);
            }
        }
    }

    for (int i = 0; i < contourFinder.nBlobs; i++) {
        ofRectangle rect = contourFinder.blobs[i].boundingRect;
        
        if (isNeonEnabled) {  // Applica effetto neon se attivato
            drawNeonRectangle(rect, drawColor, glowTransparencySlider, glowThicknessSlider);
        } else {
            ofSetColor(drawColor);
            ofNoFill();  // Assicura che i rettangoli dei contorni non siano pieni
            ofSetLineWidth(rectThicknessSlider);
            ofDrawRectangle(rect);
        }
    }

    if (randomPoints.size() > 1) {
        for (int i = 1; i < randomPoints.size(); i++) {
            if (isNeonEnabled) {
                drawNeonEffect(randomPoints[i-1], randomPoints[i], drawColor, glowTransparencySlider, glowThicknessSlider);
            } else {
                ofSetColor(drawColor);
                ofSetLineWidth(rectThicknessSlider);
                ofDrawLine(randomPoints[i-1], randomPoints[i]);
            }
        }
    }

    // Verifica della soglia sonora e disegno della linea centrale e dei rettangoli paralleli
    float soundThreshold = soundThresholdSlider;
    if (scaledVol > soundThreshold) {
        int initialLineHeight = 20;
        int initialWidth = newWidth;
        int spacing = 10;  // Spaziatura costante di 10 pixel tra i rettangoli
        int interlinea = interlineaSlider;  // Ottieni il valore di interlinea dal nuovo slider
        int yCenter = (newHeight / 2) - (initialLineHeight / 2);  // Posizionato al centro dell'immagine allocata

        // Rettangolo centrale pieno con il colore "Mov" e senza contorno
        ofSetColor(drawColor);
        ofFill();
        ofDrawRectangle(0, yCenter, initialWidth, initialLineHeight);

        // Disegna i rettangoli aggiuntivi sopra e sotto
        int numRectangles = (scaledVol - soundThreshold) / 2;  // Calcola quanti rettangoli in base al volume
        int lineHeight = initialLineHeight;
        int rectWidth = initialWidth;

        for (int i = 1; i <= numRectangles; i++) {
            // Riduci altezza per ogni rettangolo, riduci larghezza solo in modalità sfera
            lineHeight = std::max(2, lineHeight - 2);  // Altezza minima 2 pixel
            if (isSphereMode) {
                rectWidth = std::max(10, rectWidth - 10);  // Larghezza minima 10 pixel in modalità sfera
            }

            int offset = i * (lineHeight + interlinea + spacing);

            // Rettangolo sopra
            ofDrawRectangle((initialWidth - rectWidth) / 2, yCenter - offset, rectWidth, lineHeight);

            // Rettangolo sotto
            ofDrawRectangle((initialWidth - rectWidth) / 2, yCenter + offset, rectWidth, lineHeight);
        }

        // Disegna gli elementi interni di ogni rettangolo con il colore "Sfondo"
        lineHeight = initialLineHeight;
        rectWidth = initialWidth;
        for (int i = 0; i <= numRectangles; i++) {
            int offset = i * (lineHeight + interlinea + spacing);
            int yPositions[3] = { yCenter, yCenter - offset, yCenter + offset };

            for (int j = 0; j < 3; j++) {
                int yPosition = yPositions[j];
                ofSetColor(backgroundColor);

                for (int y = yPosition; y < yPosition + lineHeight; y++) {
                    for (int x = (initialWidth - rectWidth) / 2; x < (initialWidth + rectWidth) / 2; x++) {
                        if (pixels[y * diffImage.getWidth() + x] > 0) {
                            ofDrawRectangle(x, y, 1, 1);  // Disegna il pixel in movimento
                        }
                    }
                }

                // Colore inverso per i rettangoli di contorno e le linee random
                for (int k = 0; k < contourFinder.nBlobs; k++) {
                    ofRectangle rect = contourFinder.blobs[k].boundingRect;
                    if (rect.y + rect.height >= yPosition && rect.y <= yPosition + lineHeight) {
                        ofNoFill();
                        ofSetLineWidth(rectThicknessSlider);
                        ofDrawRectangle(rect);  // Disegna il rettangolo nel colore "Sfondo" senza riempimento
                    }
                }

                if (randomPoints.size() > 1) {
                    for (int k = 1; k < randomPoints.size(); k++) {
                        if (randomPoints[k - 1].y >= yPosition && randomPoints[k].y <= yPosition + lineHeight) {
                            ofSetLineWidth(rectThicknessSlider);
                            ofDrawLine(randomPoints[k - 1], randomPoints[k]);  // Linea nel colore "Sfondo"
                        }
                    }
                }
            }
            lineHeight = std::max(2, lineHeight - 2);
            if (isSphereMode) {
                rectWidth = std::max(10, rectWidth - 10);
            }
        }
    }

    ofSetColor(drawColor);
    ofDrawBitmapString("mov: " + ofToString(movingPixelCount), 10, 20);
    ofDrawBitmapString("vicinanza: " + ofToString(minDistance), 10, 40);
}

//--------------------------------------------------------------
void ofApp::drawGUI(ofEventArgs& e) {
    this->guiWindow->makeCurrent();
    ofClear(240);
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input){
    float curVol = 0.0;

    int numCounted = 0;
    for (size_t i = 0; i < input.getNumFrames(); i++) {
        curVol += input[i] * input[i];
        numCounted++;
    }

    curVol /= (float)numCounted;
    curVol = sqrt(curVol);

    smoothedVol *= 0.93;
    smoothedVol += 0.07 * curVol;

    scaledVol = 20.0 * log10(smoothedVol);
}

//--------------------------------------------------------------
// Funzione di supporto per creare l'effetto neon migliorato con più strati
void ofApp::drawNeonEffect(const ofPoint& point1, const ofPoint& point2, const ofColor& color, float transparency, float thickness) {
    // Strato esterno 1
    ofSetColor(color, transparency / 6);
    ofSetLineWidth(thickness + 18);
    ofDrawLine(point1, point2);

    // Strato esterno 2
    ofSetColor(color, transparency / 4);
    ofSetLineWidth(thickness + 12);
    ofDrawLine(point1, point2);

    // Strato medio
    ofSetColor(color, transparency / 2);
    ofSetLineWidth(thickness + 8);
    ofDrawLine(point1, point2);

    // Strato interno
    ofSetColor(color, transparency * 0.75);
    ofSetLineWidth(thickness + 4);
    ofDrawLine(point1, point2);

    // Linea principale (più luminosa e sottile)
    ofSetColor(color);
    ofSetLineWidth(thickness);
    ofDrawLine(point1, point2);
}

void ofApp::drawNeonRectangle(const ofRectangle& rect, const ofColor& color, float transparency, float thickness) {
    // Strato esterno 1
    ofSetColor(color, transparency / 6);
    ofSetLineWidth(thickness + 18);
    ofNoFill();
    ofDrawRectangle(rect);

    // Strato esterno 2
    ofSetColor(color, transparency / 4);
    ofSetLineWidth(thickness + 12);
    ofDrawRectangle(rect);

    // Strato medio
    ofSetColor(color, transparency / 2);
    ofSetLineWidth(thickness + 8);
    ofDrawRectangle(rect);

    // Strato interno
    ofSetColor(color, transparency * 0.75);
    ofSetLineWidth(thickness + 4);
    ofDrawRectangle(rect);

    // Rettangolo principale
    ofSetColor(color);
    ofSetLineWidth(thickness);
    ofDrawRectangle(rect);
}


//--------------------------------------------------------------
void ofApp::setBackgroundWhite() {
    backgroundColor = ofColor(255);
}

//--------------------------------------------------------------
void ofApp::setBackgroundGray() {
    backgroundColor = ofColor(128);
}

//--------------------------------------------------------------
void ofApp::setBackgroundBlack() {
    backgroundColor = ofColor(0);
}

//--------------------------------------------------------------
void ofApp::setColorRed() {
    drawColor = ofColor(255, 0, 0);
}

//--------------------------------------------------------------
void ofApp::setColorYellow() {
    drawColor = ofColor(255, 255, 0);
}

//--------------------------------------------------------------
void ofApp::setColorPurple() {
    drawColor = ofColor(128, 0, 128);
}

//--------------------------------------------------------------
void ofApp::foxShadowButtonPressed() {
    isFoxShadowMode = !isFoxShadowMode;
}

//--------------------------------------------------------------
void ofApp::neonButtonPressed() {
    isNeonEnabled = !isNeonEnabled;  // Attiva/disattiva l'effetto neon
}
void ofApp::blackAndWhiteButtonPressed() {
    isBlackAndWhite = !isBlackAndWhite; // Inverti lo stato
}
