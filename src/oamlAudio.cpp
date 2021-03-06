//-----------------------------------------------------------------------------
// Copyright (c) 2015-2016 Marcelo Fernandez
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <libgen.h>
#endif
#include <math.h>

#include "oamlCommon.h"


oamlAudio::oamlAudio(oamlFileCallbacks *cbs, bool _verbose) {
	verbose = _verbose;
	fcbs = cbs;

	type = 0;
	bars = 0;

	samplesCount = 0;
	samplesPerSec = 0;
	samplesToEnd = 0;
	totalSamples = 0;

	bpm = 0;
	beatsPerBar = 0;
	minMovementBars = 0;
	randomChance = 0;

	fadeIn = 0;
	fadeInSamples = 0;

	fadeOut = 0;
	fadeOutSamples = 0;
	fadeOutStart = 0;

	xfadeIn = 0;
	xfadeOut = 0;

	condId = 0;
	condType = 0;
	condValue = 0;
	condValue2 = 0;

	pickable = true;
}

oamlAudio::~oamlAudio() {
}

void oamlAudio::SetCondition(int id, int type, int value, int value2) {
	condId = id;
	condType = type;
	condValue = value;
	condValue2 = value2;
}

bool oamlAudio::TestCondition(int id, int value) {
	if (id != condId)
		return false;

	switch (condType) {
		case OAML_CONDTYPE_EQUAL:
			if (value == condValue)
				return true;
			break;

		case OAML_CONDTYPE_GREATER:
			if (value > condValue)
				return true;
			break;

		case OAML_CONDTYPE_LESS:
			if (value < condValue)
				return true;
			break;

		case OAML_CONDTYPE_RANGE:
			if (value >= condValue && value <= condValue2)
				return true;
			break;
	}

	return false;
}

unsigned int oamlAudio::GetBarsSamples(int bars) {
	if (bpm == 0)
		return 0;

	float secs = bars * (60.f / bpm) * beatsPerBar;
	return (unsigned int)(secs * samplesPerSec);
}

oamlRC oamlAudio::Load() {
	oamlRC ret = Open();
	if (ret != OAML_OK) return ret;

	for (std::vector<oamlLayer>::iterator layer=layers.begin(); layer<layers.end(); ++layer) {
		ret = layer->Load();
		if (ret != OAML_OK)
			return ret;
	}

	return OAML_OK;
}

oamlRC oamlAudio::Open() {
	if (verbose) __oamlLog("%s %s\n", __FUNCTION__, GetFilenameStr());

	for (std::vector<oamlLayer>::iterator layer=layers.begin(); layer<layers.end(); ++layer) {
		oamlRC ret = layer->Open();
		if (ret != OAML_OK)
			return ret;

		if (totalSamples == 0) {
			channelCount = layer->GetChannels();
			samplesPerSec = layer->GetSamplesPerSec();
			totalSamples = layer->GetTotalSamples();
		}
	}

	samplesToEnd = GetBarsSamples(bars);
	if (samplesToEnd == 0) {
		samplesToEnd = totalSamples;
	}

	for (std::vector<oamlLayer>::iterator layer=layers.begin(); layer<layers.end(); ++layer) {
		layer->SetSamplesToEnd(samplesToEnd);
	}

	samplesCount = 0;
	fadeInSamples = 0;

	if (fadeOut) {
		fadeOutSamples = (unsigned int)((fadeOut / 1000.f) * samplesPerSec);
		fadeOutStart = samplesToEnd - fadeOutSamples;
	} else {
		fadeOutSamples = 0;
		fadeOutStart = 0;
	}

	return OAML_OK;
}

void oamlAudio::DoFadeIn(int msec) {
	fadeInSamples = (unsigned int)((msec / 1000.f) * samplesPerSec);
}

void oamlAudio::DoFadeOut(int msec) {
	fadeOutStart = samplesCount;
	fadeOutSamples = (unsigned int)((msec / 1000.f) * samplesPerSec);
}

bool oamlAudio::HasFinished() {
	// Check if we're fading out
	if (fadeOutSamples) {
		unsigned int fadeOutFinish = fadeOutStart + fadeOutSamples;
		if (samplesCount >= fadeOutFinish) {
			// Fade out finished so we're done
			return true;
		}
	}

	// Check if our samples reached our end (based off the number of bars)
	return samplesCount >= samplesToEnd;
}

bool oamlAudio::HasFinishedTail(unsigned int pos) {
	return pos >= totalSamples;
}

float oamlAudio::ReadFloat() {
	float sample = 0.f;

	for (std::vector<oamlLayer>::iterator layer=layers.begin(); layer<layers.end(); ++layer) {
		sample+= layer->ReadFloat(samplesCount);
	}

	if (fadeInSamples) {
		if (samplesCount < fadeInSamples) {
			float gain = 1.f - float(fadeInSamples - samplesCount) / float(fadeInSamples);
			sample*= gain;
		} else {
			fadeInSamples = 0;
		}
	}

	if (fadeOutSamples && samplesCount >= fadeOutStart) {
		unsigned int fadeOutFinish = fadeOutStart + fadeOutSamples;
		if (samplesCount < fadeOutFinish) {
			float gain = float(fadeOutFinish - samplesCount) / float(fadeOutSamples);
			sample*= gain;
		} else {
			sample = 0.f;
		}
	}

	samplesCount++;

	return sample;
}

float oamlAudio::ReadFloat(unsigned int pos) {
	float sample = 0.f;

	if (pos > totalSamples)
		return 0;

	for (std::vector<oamlLayer>::iterator layer=layers.begin(); layer<layers.end(); ++layer) {
		sample+= layer->ReadFloat(pos, true);
	}

	return sample;
}

void oamlAudio::SetFilename(std::string audioFilename, std::string layer, oamlLayerInfo *info) {
	layers.push_back(oamlLayer(audioFilename, layer, info, fcbs, verbose));

	filename = audioFilename;
	size_t pos = filename.find_last_of(PATH_SEPARATOR);
	if (pos != std::string::npos) {
		name = filename.substr(pos+1);
		size_t pos = name.find_last_of('.');
		if (pos != std::string::npos) {
			name = name.substr(0, pos);
		}
	}
}

void oamlAudio::ReadSamples(float *samples, int channels) {
	if (channelCount == 1) {
		// Mono audio to mono/stereo output
		float sample = ReadFloat();

		for (int i=0; i<channels; i++) {
			samples[i] = sample;
		}
	} else if (channelCount == 2) {
		if (channels == 1) {
			// Stereo audio to mono output
			float left = ReadFloat();
			float right = ReadFloat();

			samples[0] = left * 0.5f + right + 0.5f;
		} else if (channels == 2) {
			// Stereo audio to stereo output
			for (int i=0; i<channels; i++) {
				samples[i] = ReadFloat();
			}
		}
	}
}

unsigned int oamlAudio::ReadSamples(float *samples, int channels, unsigned int pos) {
	if (channelCount == 1) {
		float sample = ReadFloat(pos++);

		for (int i=0; i<channels; i++) {
			samples[i] = sample;
		}
	} else if (channelCount == 2) {
		if (channels == 1) {
			float left = ReadFloat(pos++);
			float right = ReadFloat(pos++);

			samples[0] = left * 0.5f + right + 0.5f;
		} else if (channels == 2) {
			for (int i=0; i<channels; i++) {
				samples[i] = ReadFloat(pos++);
			}
		}
	}

	return pos;
}
