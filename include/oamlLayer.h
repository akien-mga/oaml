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

#ifndef __OAMLLAYER_H__
#define __OAMLLAYER_H__

class ByteBuffer;

typedef struct {
	int id;
	std::string name;
	int randomChance;
	float gain;
} oamlLayerInfo;

class oamlLayer {
private:
	bool verbose;
	oamlFileCallbacks *fcbs;

	ByteBuffer buffer;
	audioFile *handle;
	oamlLayerInfo *info;
	std::string filename;
	std::string layer;

	unsigned int bytesPerSample;
	unsigned int samplesPerSec;
	unsigned int totalSamples;
	unsigned int channelCount;
	unsigned int samplesToEnd;

	bool chance;
	bool lastChance;

	oamlRC OpenFile();

	int Read();
	int Read32(unsigned int pos);

public:
	oamlLayer(std::string _filename, std::string _layer, oamlLayerInfo *_info, oamlFileCallbacks *cbs, bool _verbose);
	~oamlLayer();

	void SetFilename(std::string layerFilename) { filename = layerFilename; }

	std::string GetFilename() const { return filename; }
	const char *GetFilenameStr() const { return filename.c_str(); }

	oamlRC Open();
	oamlRC Load();
	float ReadFloat(unsigned int pos, bool isTail = false);

	unsigned int GetChannels() const { return channelCount; }
	unsigned int GetTotalSamples() const { return totalSamples; }
	unsigned int GetSamplesPerSec() const { return samplesPerSec; }
	void SetSamplesToEnd(unsigned int samples) { samplesToEnd = samples; }
};

#endif
