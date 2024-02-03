/**
*********************************************************************
*********
* @project_name :MusicClion
* @file : AudioFileParser.cpp
* @author : zen3
* @brief : None
* @attention : None
* @date : 2024/1/30 
*********************************************************************
*********
*/
//

#include "AudioFileParser.h"

AudioFileParser::~AudioFileParser() {

}

const uint8_t *AudioFileParser::getTitle() const {
    return title;
}

const uint8_t *AudioFileParser::getArtist() const {
    return artist;
}

uint32_t AudioFileParser::getTotsec() const {
    return totsec;
}

void AudioFileParser::setTotsec(uint32_t totsec) {
    AudioFileParser::totsec = totsec;
}

uint32_t AudioFileParser::getCursec() const {
    return cursec;
}

void AudioFileParser::setCursec(uint32_t cursec) {
    AudioFileParser::cursec = cursec;
}

uint32_t AudioFileParser::getBitrate() const {
    return bitrate;
}

void AudioFileParser::setBitrate(uint32_t bitrate) {
    AudioFileParser::bitrate = bitrate;
}

uint32_t AudioFileParser::getSamplerate() const {
    return samplerate;
}

void AudioFileParser::setSamplerate(uint32_t samplerate) {
    AudioFileParser::samplerate = samplerate;
}

uint32_t AudioFileParser::getDatastart() const {
    return datastart;
}

void AudioFileParser::setDatastart(uint32_t datastart) {
    AudioFileParser::datastart = datastart;
}
