/***************************************************************************
 *   Copyright (C) 2005 - 2007 by                                          *
 *      Last.fm Ltd <mir@last.fm>                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef __MP3_SOURCE
#define __MP3_SOURCE

#include <string>
#include <vector>
#include <fstream>

#include <mad.h>

#include <QString>
#include <QFile>

using namespace std;

// ----------------------------------------------------------------------- ------

class MP3_Source
{
public:

   // ctor
   MP3_Source();
   virtual ~MP3_Source();

   static void getInfo(const QString& fileName, int& lengthSecs, int& samplerate, int& bitrate, int& nchannels);
   virtual void  init(const QString& fileName);
   virtual void  release();

   // return a chunk of PCM data from the mp3
   virtual int updateBuffer(signed short* pBuffer, size_t bufferSize);

   virtual void skip(const int mSecs);
   virtual void skipSilence(double silenceThreshold = 0.0001);

   bool  eof() const { return m_inputFile.atEnd(); }

private:

   static bool fetchData( QFile& mp3File,
                          unsigned char* pMP3_Buffer,
                          const int MP3_BufferSize,
                          mad_stream& madStream );

   static bool isRecoverable(const mad_error& error, bool log = false);

   static string MadErrorString(const mad_error& error);

   struct mad_stream	m_mad_stream;
   struct mad_frame	    m_mad_frame;
   mad_timer_t			m_mad_timer;
   struct mad_synth	    m_mad_synth;

   QString              m_fileName;
   QFile                m_inputFile;

   unsigned char*       m_pMP3_Buffer;
   static const int     m_MP3_BufferSize = (5*8192);

   size_t               m_pcmpos;
};

// ---------------------------------------------------------------------

// -----------------------------------------------------------------------------

#endif // __MP3_SOURCE
