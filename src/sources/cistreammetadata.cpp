/*
 *    Copyright (C) 2022
 *    Dr. Sven Alisch (svenali@gmx.de)
 *
 *    This file is part of the waverider.
 *    waverider is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    waverider is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with waverider; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
#include "cistreammetadata.h"

#define AUDIOBUFFERSIZE 32768

int readFromRingBuffer(void* opaque, uint8_t* buf, int buf_size)
{
#if defined(HAVE_FFMPEG)
    struct TStreamingBuffer* sb = reinterpret_cast<TStreamingBuffer*>(opaque);
    //RingBuffer<uint8_t>* rb = reinterpret_cast<RingBuffer<uint8_t>*>(opaque);
    
    buf_size = FFMIN(buf_size, sb->b_size);
    //int size = rb->GetRingBufferReadAvailable();
    /* cerr << size << " Bytes in RingBuffer available." << endl;
    buf_size = FFMIN(buf_size, size);
    cerr << "Read " << buf_size << " Bytes into Buffer." << endl; */
    
    if (!buf_size)
         return AVERROR_EOF;
    
    printf("Pointer: %p, Buffer_Size of RB: %zu, buf_size(callback): %i\n", sb->buf, sb->b_size, buf_size);

    memcpy(buf, sb->buf, buf_size);          // Because of seek
    sb->buf += buf_size;
    sb->b_size -= buf_size;

    cerr << "Space in Buffer: " << sb->b_size << endl;

    //buf_size = rb->getDataFromBuffer(buf, buf_size);
    
    return buf_size;
#endif
}

/* static int64_t seekInStreamBuffer(void *opaque, int64_t offset, int whence)
{
    cerr << "seekInStreamBuffer" << endl;
#if defined(HAVE_FFMPEG)
    //cerr << "SeekType: " << whence << endl;
    //cerr << "SEEKING TYPES ARE: " << SEEK_SET << "; " << SEEK_CUR << "; " << SEEK_END << endl;
    TStreamingBuffer* bd = reinterpret_cast<TStreamingBuffer*>(opaque);
    
    switch (whence)
    {
        case SEEK_SET:
            cerr << "Seek set to " << offset << endl;
            bd->ptr = bd->buf + offset;
            bd->space_in_buffer = bd->b_size - offset;
            cerr << "Space in Buffer: " << bd->space_in_buffer << endl;
            return (int64_t) bd->ptr;
            break;
        case SEEK_CUR:
            cerr << "Seek cur to " << offset << endl;
            bd->ptr += offset;
            bd->space_in_buffer -= offset;
            break;
        case SEEK_END:
            cerr << "Seek to end " << offset << endl;
            bd->ptr = (bd->buf + bd->b_size) + offset;
            bd->space_in_buffer = 0;
            return (int64_t) bd->ptr;
            break; 
        case AVSEEK_SIZE:
            cerr << "Seek Size: " << bd->b_size << endl;
            return (int64_t) bd->b_size;
            break;
        
        default:
            return -1;
    }
    
    return 1;
#endif
}*/

CIStreamMetaData::CIStreamMetaData(CRadioController* r) 
    : _radioController(r),
      _metaDataBuffer(32 * AUDIOBUFFERSIZE)
{
    _running = false;
    _fp = nullptr;
#if defined(HAVE_FFMPEG)
    _FormatContext = NULL;
#endif
}

CIStreamMetaData::~CIStreamMetaData()
{

}

void CIStreamMetaData::init_format_context()
{
#if defined(HAVE_FFMPEG)
    cerr << "Init Format Context for MetaData Analysis ..." << endl;
    AVIOContext *output_io_context = NULL;
    
    int output_buffer_size = 4096;
    uint8_t *output_buffer = (uint8_t*) av_malloc(output_buffer_size);

    int error;
    
    output_io_context = avio_alloc_context(
        output_buffer, 
        output_buffer_size, 
        0, 
        reinterpret_cast<void*>(&_streamingBuffer), 
        //reinterpret_cast<void*>(&_metaDataBuffer),
        &readFromRingBuffer, 
        nullptr, 
        nullptr); //&seekInStreamBuffer);
    
    /* Create a new format context for the output container format. */
    if (!(_FormatContext = avformat_alloc_context()))    
    {
        cerr << "Could not allocate output format context" << endl;
        exit(AVERROR(ENOMEM));
    }
 
    /* Associate the output file (pointer) with the container format context. */
    _FormatContext->pb = output_io_context;
    //_FormatContext->flags = AVFMT_FLAG_CUSTOM_IO;

    cerr << "Format Context created ..." << endl;
#endif
}
        
void CIStreamMetaData::start_analysing()
{
    if (!_running)
    {
        _running = true;
        _mainThread = std::thread(&CIStreamMetaData::analyseThread, this);
    }
}

void CIStreamMetaData::stop_analysing()
{
    _running = false;
}

void CIStreamMetaData::analyseThread()
{
#if defined(HAVE_FFMPEG)
    const AVDictionaryEntry *tag = NULL;
    int buffer_size = 10 * AUDIOBUFFERSIZE;
    uint8_t* buffer = (uint8_t*) av_malloc(buffer_size);
    
    int ret = 0;

    bool wait_for_data = true;
    do {
        if (_metaDataBuffer.GetRingBufferReadAvailable() > 4 * AUDIOBUFFERSIZE)
        {
                wait_for_data = false;
        }
        cerr << "analyseThread: Wait for Data -> " << _metaDataBuffer.GetRingBufferReadAvailable() << " Bytes." << endl;
        this_thread::sleep_for(chrono::seconds(1));
    } while (wait_for_data); 

    int size = _metaDataBuffer.GetRingBufferReadAvailable();
    int load = FFMIN(buffer_size, size);
    cerr << load << " Bytes ready..." << endl;
    _metaDataBuffer.getDataFromBuffer(buffer, load);
    _streamingBuffer.buf = buffer;
    _streamingBuffer.b_size = load;

    if (ret == 0)
    {
        while (_running)
        {
            init_format_context();

            ret = avformat_open_input(&_FormatContext, NULL, NULL, NULL);

            if (ret == 0)
            {
                cerr << "CIStreamMetaData: Stream opened ... " << endl;
            }
            else
            {
                char desc[255];
                av_strerror(ret, desc, 255);
                printf("libav Error: %s\n", desc);
                cerr << " No luck to open stream ... Buffer: " << _metaDataBuffer.GetRingBufferReadAvailable() << " Bytes in RingBuffer " << endl;
                
                break;
            }

            if ((ret = avformat_find_stream_info(_FormatContext, NULL)) < 0) 
            {
                av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
            }  

            cerr << "Find Stream returned with " << ret << endl;

            if (ret >= 0)
            {
                while ((tag = av_dict_get(_FormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                {
                    cerr << "[CISStreamMetaData] Key: " << tag->key << " Value: " << tag->value << endl;
                }
            }    
            cerr << "REFILL THE BUFFER" << endl;

            //Refill buffer
            do
            {
                size = _metaDataBuffer.GetRingBufferReadAvailable();
                load = FFMIN(buffer_size, size);

                cerr << size << " Bytes loading ...";
                this_thread::sleep_for(chrono::seconds(1));
            } while (load < 4 * AUDIOBUFFERSIZE);    

            avformat_close_input(&_FormatContext);

            _metaDataBuffer.getDataFromBuffer(buffer, load);
            cerr << " ... loaded." << endl;
            _streamingBuffer.buf = buffer;
            _streamingBuffer.b_size = load;
        }
    }      
#endif
}

void CIStreamMetaData::copyToAnalyseBuffer(uint8_t* data, int len)
{
    /* if (!_fp)
    {
        _fp = fopen("test-for-analyse.mp3", "wb");
    }
    
    fwrite(data, len, 1, _fp); */

    _metaDataBuffer.putDataIntoBuffer(data, len);
}