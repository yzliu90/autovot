#include "WavFile.h"
#include <stdio.h>
#include <iostream>

/********************************************************************
 Function:    CWavFile()
 Author:      Yossi Keshet
 Description: constructor
 Inputs:      none.
 Outputs:     none.
 Comments:    none.
 ********************************************************************/
CWavFile::CWavFile()
{
}

CWavFile::~CWavFile()
{
}


/********************************************************************
 Function:    Close()
 Author:      Yossi Keshet
 Description: close a WAV file
 Inputs:      none.
 Outputs:     void.
 Comments:    none.
 ********************************************************************/
void CWavFile::Close()
{
	fclose(m_pfile);
}

/********************************************************************
 Function:    Open()
 Author:      Yossi Keshet
 Description: open a WAV file
 Inputs:      string filename
 Outputs:     bool - true if file was sucessufly openned, otherwise false
 Comments:    none.
 ********************************************************************/
bool CWavFile::Open(std::string filename)
{
	m_filename = filename;
	m_pfile = fopen(m_filename.c_str(), "rb");
	if (m_pfile == NULL)
		return false;
	return true;
}

/********************************************************************
 Function:    Read()
 Author:      Yossi Keshet
 Description: read bytes from file
 Inputs:      vBuffer - buffer which be filled with new data
              nBufferLength - buffer length
 Outputs:     The number of byets read.
 Comments:    Should be used only internally (private function).
 ********************************************************************/
unsigned long CWavFile::Read(void *vBuffer, unsigned long nBufferLength)
{
	return fread(vBuffer, sizeof(char), nBufferLength, m_pfile);
}

/********************************************************************
 Function:    Seek()
 Author:      Yossi Keshet
 Description: Sets the file position indicator
 Inputs:      lOff - offset
              nFrom - SEEK_SET, SEEK_CUR, or SEEK_END
 Outputs:     return the value 0 if successful, otherwise -1.
 Comments:    Should be used only internally (private function).
 ********************************************************************/
unsigned long CWavFile::Seek(unsigned long lOff, unsigned long nFrom)
{
	return fseek(m_pfile, lOff, nFrom);
}

/********************************************************************
 Function:    GetRate()
 Author:      Yossi Keshet
 Description: return the sampling rate
 Inputs:      none.
 Outputs:     Sampling rate
 Comments:    none.
 ********************************************************************/
unsigned long CWavFile::GetRate()
{
	if (m_pfile == NULL)
		return 0;
	return m_PcmWaveFormat.dwSamplesPerSec;
}

/********************************************************************
 Function:    GetFrameCount()
 Author:      Yossi Keshet
 Description: Return the number of samples in the file
 Inputs:      none.
 Outputs:     The number of samples in the file.
 Comments:    none.
 ********************************************************************/
unsigned long CWavFile::GetFrameCount()
{
	return m_ulDataLength;
}

/********************************************************************
 Function:    LoadSamples()
 Author:      Yossi Keshet
 Description: Load samples from WAV file 
 Inputs:      sBuffer - buffer which be filled with new raw samples
              nBufferLength - buffer length
 Outputs:     The number of samples transferred to the buffer.
 Comments:    The buffer should be allocated ahead of calling this 
              function.
 ********************************************************************/
unsigned long CWavFile::LoadSamples(short *sBuffer, unsigned long nBufferLength)
{
	unsigned long nBytesRead = 0;
	nBytesRead = Read((void *)sBuffer, nBufferLength*sizeof(short));
	nBytesRead = int(nBytesRead/float(sizeof(short)));
	return nBytesRead;
}


/********************************************************************
 Function:    ReadHeader()
 Author:      Yossi Keshet
 Description: Fill the WAVFORMAT structure with the WAV file params.
 Inputs:      void.
 Outputs:     true for success, false for error.
 Comments:    none.
 ********************************************************************/
unsigned long CWavFile::ReadHeader (void)
{
	char szString[5];

	// Read Chunk type
	Read(szString, 4);
	szString[4] = '\0';
	if (strcmp(szString, "RIFF"))
		return false;

	// Seek over Total file size minus eight
	Seek(4, SEEK_CUR);

	// Read Form name
	Read(szString, 4);
	szString[4] = '\0';
	if (strcmp(szString, "WAVE"))
		return false;

	// Read Chunk type
	Read(szString, 4);
	szString[4] = '\0';
	if (strcmp(szString, "fmt "))
		return false;

	// Read Format chunk data length
	UINT32 ulFmtChkDatLength;
	Read(&ulFmtChkDatLength, sizeof(UINT32));

	// Read Format chunk data
	Read(&m_PcmWaveFormat, sizeof(PCMWAVEFORMAT));

	// Check for supported formats
	if (m_PcmWaveFormat.wFormatTag != 0x0001) {
		std::cerr << "Error: " << m_filename << " has wrong format. Should be uncompressed PCM." << std::endl;
		return false;
	}
	
	if (m_PcmWaveFormat.wChannels != 1) {
		std::cerr << "Error: " << m_filename << " has wrong format. Should have only one channel." << std::endl;
		return false;
	}

	if (m_PcmWaveFormat.dwSamplesPerSec != 16000) {
		std::cerr << "Error: " << m_filename << " has wrong format. Sampling rate should be 16kHz and not "
		<< m_PcmWaveFormat.dwSamplesPerSec << "." << std::endl;
		return false;
	}

	if (m_PcmWaveFormat.wBitsPerSample != 16) {
		std::cerr << "Error: " << m_filename << " has wrong format. Each sample should have 16 bits, not "
		<< m_PcmWaveFormat.wBitsPerSample << " bits." << std::endl;
		return false;
	}

	// Skip unrellevant data
	Seek(ulFmtChkDatLength-sizeof(PCMWAVEFORMAT), SEEK_CUR);

	// Read untill chunk type is 'data'
	Read(szString, 4);
	szString[4] = '\0';
	while (strcmp(szString, "data") != 0)
	{
		if (Read(szString, 4) < 4)
			return false;
		szString[4] = '\0';
	}

	// Read length of sound data
	Read(&m_ulDataLength, sizeof(UINT32));
	
	return int(m_ulDataLength*8.0/float(m_PcmWaveFormat.wBitsPerSample));
}
