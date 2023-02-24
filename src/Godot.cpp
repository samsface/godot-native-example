#include <Godot.hpp>
#include <StreamPeerBuffer.hpp>
#include <Reference.hpp>
#include <memory>
#include <set>
#include <taglib.h>
#include <taglib/fileref.h>
#include <id3v2tag.h>
#include <id3v2header.h>
#include <id3v2frame.h>
#include <mpegfile.h>
#include <attachedpictureframe.h>
#include <audioproperties.h>
#include <mp4file.h>
#include <mp4tag.h>
#include <mp4coverart.h>
#include <vorbisproperties.h>
#include <wavproperties.h>
#include <mp4properties.h>
#include <speexproperties.h>
#include <opusproperties.h>
#include <apeproperties.h>
#include <flacproperties.h>
#include <audioproperties.h>
#include <flacfile.h>
#include <flacpicture.h>
#include <mp4properties.h>
#include <vorbisfile.h>
#include <synchronizedlyricsframe.h>
#include <unsynchronizedlyricsframe.h>
#include <popularimeterframe.h>
#include <tpropertymap.h>


TagLib::String gd_2_taglib_string(godot::String g_string) {
    return g_string.alloc_c_string();
}


godot::PoolByteArray ByteVector2PoolByte(TagLib::ByteVector data) {
    godot::PoolByteArray x;
    for (size_t i = 0; i < data.size(); i++) {
        x.push_back(data[i]);
    }
    return x;
}


TagLib::FileName gd_string_to_filename(godot::String file_path) {
#ifdef _WIN32
    return TagLib::FileName(file_path.unicode_str());
#endif //
    return TagLib::FileName(file_path.alloc_c_string());
}


static TagLib::String get_extension(TagLib::String file_path) {
    TagLib::String x = file_path.substr(file_path.size() - 3);
    x = x.upper();
    return  x;
}


bool save_buffer_to_file(TagLib::String dst_path, TagLib::ByteVector data) {
    FILE* fout = NULL;
    fout = fopen(dst_path.toCString(true), "wb");
    if (!fout) {
        return false;
    }
    fwrite(data.data(), data.size(), 1, fout);
    fclose(fout);
    return true;
}


TagLib::FLAC::Picture* create_flac_picture(TagLib::ByteVector image_data, TagLib::String mime_type) {
    TagLib::FLAC::Picture* new_picture = new TagLib::FLAC::Picture;
    new_picture->setData(image_data);
    new_picture->setMimeType(mime_type);
    return new_picture;
}


class ImageFile : public TagLib::File
{
public:

    ImageFile(TagLib::FileName file) : TagLib::File(file)
    {

    }

    TagLib::ByteVector data()
    {
        return readBlock(length());
    }


private:
    virtual TagLib::Tag* tag() const { return 0; }
    virtual TagLib::AudioProperties* audioProperties() const { return 0; }
    virtual bool save() { return false; }
};


class AudioProperties : public godot::Reference {
    GODOT_CLASS(AudioProperties, godot::Reference)

public:
    static void _register_methods() {
        register_method("get_duration_seconds", &AudioProperties::get_duration_seconds);
        register_method("get_channels", &AudioProperties::get_channels);
        register_method("get_bitrate", &AudioProperties::get_bitrate);
        register_method("get_sample_rate", &AudioProperties::get_sample_rate);
    }

    void _init()
    {
    }

    int get_duration_seconds(godot::String path) {
        TagLib::FileRef audioFile(gd_string_to_filename(path));
        if (audioFile.isNull()) {
            return 0;
        }
        //Returns length of the given Audio File in Seconds
        return audioFile.audioProperties()->lengthInSeconds();
    }

    int get_channels(godot::String path) {
        TagLib::FileRef audioFile(gd_string_to_filename(path));
        if (audioFile.isNull()) {
            return 0;
        }
        //Returns length of the given Audio File in Seconds
        return audioFile.audioProperties()->channels();
    }

    int get_bitrate(godot::String path) {
        TagLib::FileRef audioFile(gd_string_to_filename(path));
        if (audioFile.isNull()) {
            return 0;
        }
        //Returns length of the given Audio File in Seconds
        return audioFile.audioProperties()->bitrate();
    }

    int get_sample_rate(godot::String path) {
        TagLib::FileRef audioFile(gd_string_to_filename(path));
        if (audioFile.isNull()) {
            return 0;
        }
        //Returns length of the given Audio File in Seconds
        return audioFile.audioProperties()->sampleRate();
    }
};


class MPEG : public godot::Reference {
    GODOT_CLASS(MPEG, godot::Reference)

public:
    static void _register_methods() {
        register_method("get_cover", &MPEG::get_cover);
        register_method("get_covers", &MPEG::get_covers);
        register_method("get_embedded_cover_count", &MPEG::get_embedded_cover_count);
    }

    godot::PoolByteArray get_cover(godot::String filepath, unsigned int cover_idx) {
        // returns a specific cover, decided by a given index
        TagLib::MPEG::File mpeg_file(gd_string_to_filename(filepath));
        if (!mpeg_file.isOpen()) { return godot::PoolByteArray(); }
        TagLib::ID3v2::Tag* mpeg_tag = mpeg_file.ID3v2Tag(true);
        if (mpeg_tag == NULL) { return godot::PoolByteArray(); }
        
        unsigned int counter = 0;
        TagLib::ID3v2::FrameList embedded_covers = mpeg_tag->frameListMap()["APIC"];
        for (TagLib::ID3v2::FrameList::ConstIterator it = embedded_covers.begin(); it != embedded_covers.end(); it++) {
            if (counter == cover_idx) {
                return ByteVector2PoolByte((*it)->render());
            }
            counter += 1;
        }
        return godot::PoolByteArray();
    }

    godot::Array get_covers(godot::String filepath) {
        // returns the data of all attached covers as an Array of ByteArrays
        godot::Array cover_data = godot::Array();
        TagLib::MPEG::File mpeg_file(gd_string_to_filename(filepath));
        TagLib::ID3v2::Tag * mpeg_tag = mpeg_file.ID3v2Tag(true);
        if (mpeg_tag != NULL) {
            TagLib::ID3v2::FrameList embedded_covers = mpeg_tag->frameListMap()["APIC"];
            for (TagLib::ID3v2::FrameList::ConstIterator it = embedded_covers.begin(); it != embedded_covers.end(); it++) {
                cover_data.push_back(ByteVector2PoolByte((*it)->render()));
            }
        }
        return cover_data;
    }

    unsigned int get_embedded_cover_count(godot::String filepath) {
        // returns the amount of embedded covers within file
        TagLib::MPEG::File mpeg_file(gd_string_to_filename(filepath));
        if (!mpeg_file.isOpen()) { return  0; }
        if (!mpeg_file.hasID3v2Tag()) { return  0; }
        
        TagLib::ID3v2::Tag* mpeg_tag = mpeg_file.ID3v2Tag(true);
        
        return mpeg_tag->frameListMap()["APIC"].size();
    }
};


class OGG_VORBIS : public godot::Reference {
    GODOT_CLASS(OGG_VORBIS, godot::Reference)

public:
    static void _register_methods() {
        register_method("get_cover", &OGG_VORBIS::get_cover);
        register_method("get_covers", &OGG_VORBIS::get_covers);
        register_method("add_cover", &OGG_VORBIS::add_cover);
        register_method("get_embedded_cover_count", &OGG_VORBIS::get_embedded_cover_count);
    }


    void add_cover(godot::String dst_path, godot::String src_path, godot::String mime_type) {
        TagLib::Ogg::Vorbis::File ogg_file(gd_string_to_filename(dst_path));
        TagLib::Ogg::XiphComment* tag = ogg_file.tag();
        ImageFile image = ImageFile(gd_string_to_filename(src_path));
        godot::Godot::print(image.data().size());
        tag->addPicture(create_flac_picture(image.data(), mime_type.alloc_c_string()));
        ogg_file.save();
    }


    godot::PoolByteArray get_cover(godot::String filepath, unsigned int cover_idx) {
        // returns the data of the cover at the given index
        TagLib::Ogg::Vorbis::File vorbis_file(gd_string_to_filename(filepath));
        if (!vorbis_file.isOpen()) { return godot::PoolByteArray(); }

        TagLib::Ogg::XiphComment* vorbis_tag = vorbis_file.tag();
        if (!vorbis_tag) { return godot::PoolByteArray(); }

        unsigned int counter = 0;
        TagLib::List<TagLib::FLAC::Picture*> pictures = vorbis_tag->pictureList();
        for (TagLib::List<TagLib::FLAC::Picture*>::ConstIterator it = pictures.begin(); it != pictures.end(); it++) {
            if (counter == cover_idx) {
                return ByteVector2PoolByte((*it)->render());
            }
            counter += 1;
        }
        return godot::PoolByteArray();
    }


    godot::Array get_covers(godot::String filepath) {
        godot::Array cover_data = godot::Array();
        TagLib::Ogg::Vorbis::File vorbis_file(gd_string_to_filename(filepath));
        TagLib::Ogg::XiphComment* vorbis_tag = vorbis_file.tag();
        if (!vorbis_file.isOpen()) { return godot::Array(); }
        TagLib::List<TagLib::FLAC::Picture*> pictures = vorbis_tag->pictureList();
        for (TagLib::List<TagLib::FLAC::Picture*>::ConstIterator it = pictures.begin(); it != pictures.end(); it++) {
            cover_data.push_back(ByteVector2PoolByte((*it)->render()));
        }
        return cover_data;
    }


    unsigned int get_embedded_cover_count(godot::String filepath) {
        // returns the amount of embedded covers within file
        TagLib::Ogg::Vorbis::File vorbis_file(gd_string_to_filename(filepath));
        if (!vorbis_file.isOpen()) { return 0; };
        
        TagLib::Ogg::XiphComment* vorbis_tag = vorbis_file.tag();
        if (vorbis_tag == NULL) { return 0; };
        
        return vorbis_tag->pictureList().size();
    }
};


class FLAC : public godot::Reference {
    GODOT_CLASS(FLAC, godot::Reference)

public:
    static void _register_methods() {
        register_method("get_cover", &FLAC::get_cover);
        register_method("get_covers", &FLAC::get_covers);
        register_method("get_embedded_cover_count", &FLAC::get_embedded_cover_count);
    }


    godot::PoolByteArray get_cover(godot::String filepath, unsigned int cover_idx) {
        TagLib::FLAC::File flac_file(gd_string_to_filename(filepath));
        if (!flac_file.isOpen()) { return godot::PoolByteArray(); }
        
        unsigned int counter = 0;
        TagLib::List<TagLib::FLAC::Picture*> pictures = flac_file.pictureList();
        for (TagLib::List<TagLib::FLAC::Picture*>::ConstIterator it = pictures.begin(); it != pictures.end(); it++) {
            if (counter == cover_idx) {
                return ByteVector2PoolByte((*it)->render());
            }
            counter += 1;
        }

        return godot::PoolByteArray();
    }


    godot::Array get_covers(godot::String filepath) {
        godot::Array cover_data = godot::Array();
        TagLib::FLAC::File flac_file(gd_string_to_filename(filepath));
        if (flac_file.isOpen()) {
            TagLib::List<TagLib::FLAC::Picture*> pictures = flac_file.pictureList();
            for (TagLib::List<TagLib::FLAC::Picture*>::ConstIterator it = pictures.begin(); it != pictures.end(); it++) {
                cover_data.push_back(ByteVector2PoolByte((*it)->render()));
            }
        }
        return cover_data;
    }


    unsigned int get_embedded_cover_count(godot::String filepath) {
        // returns the amount of embedded covers within file
        TagLib::FLAC::File flac_file(gd_string_to_filename(filepath));
        if (!flac_file.isOpen()) { return 0; };

        return flac_file.pictureList().size();
    }

};


class MP4 : public godot::Reference {
    GODOT_CLASS(MP4, godot::Reference)

public:
    static void _register_methods() {
        register_method("get_cover", &MP4::get_cover);
        register_method("get_covers", &MP4::get_covers);
        register_method("get_embedded_cover_count", &MP4::get_embedded_cover_count);
    }

    godot::PoolByteArray get_cover(godot::String filepath, unsigned int cover_idx) {
        TagLib::MP4::File mp4_file(gd_string_to_filename(filepath));
        if (!mp4_file.isOpen()) { return godot::PoolByteArray(); }
        
        TagLib::MP4::Tag* mp4_tag = mp4_file.tag();
        if (mp4_tag == NULL) { return godot::PoolByteArray(); }

        unsigned int counter = 0;
        TagLib::MP4::CoverArtList covers = mp4_tag->itemListMap()["covr"].toCoverArtList();
        for (TagLib::MP4::CoverArtList::ConstIterator it = covers.begin(); it != covers.end(); it++) {
            if (counter == cover_idx) {
                return ByteVector2PoolByte((*it).data());
            }
            counter += 1;
        }
        return godot::PoolByteArray();
    }


    godot::Array get_covers(godot::String filepath) {
        godot::Array cover_data = godot::Array();
        TagLib::MP4::File mp4_file(gd_string_to_filename(filepath));
        if (mp4_file.isOpen() && mp4_file.hasMP4Tag()) {
            TagLib::MP4::Tag* mp4_tag = mp4_file.tag();
            TagLib::MP4::CoverArtList covers = mp4_tag->itemListMap()["covr"].toCoverArtList();
            for (TagLib::MP4::CoverArtList::ConstIterator it = covers.begin(); it != covers.end(); it++) {
                cover_data.push_back(ByteVector2PoolByte((*it).data()));
            }
        }
        return cover_data;
    }

    unsigned int get_embedded_cover_count(godot::String filepath) {
        // returns the amount of embedded covers within an MP4 tagged file
        TagLib::MP4::File mp4_file(gd_string_to_filename(filepath));
        if (!mp4_file.isOpen()) { return 0; }
        
        TagLib::MP4::Tag* mp4_tag = mp4_file.tag();
        if (mp4_tag == NULL) { return 0; }
        
        godot::Godot::print("VALID MP4TAG");
        return mp4_tag->itemListMap()["covr"].toCoverArtList().size();
    }
};


class Tagging : public godot::Reference{
    GODOT_CLASS(Tagging, godot::Reference)

public:
    static void _register_methods(){
        register_method("get_song_popularity", &Tagging::get_song_popularity);
        register_method("set_song_popularity", &Tagging::set_song_popularity);
        register_method("get_multiple_tags", &Tagging::get_multiple_tags);
        register_method("get_single_tag", &Tagging::get_single_tag);
        register_method("copy_covers", &Tagging::copy_covers);
        register_method("copy_cover", &Tagging::copy_cover);
        register_method("set_tag", &Tagging::set_tag);
        register_method("add_cover", &Tagging::add_cover);
        register_method("remove_cover", &Tagging::remove_cover);
        register_method("remove_all_covers", &Tagging::remove_all_covers);
        register_method("get_embedded_cover", &Tagging::get_embedded_cover);
        register_method("get_embedded_covers", &Tagging::get_embedded_covers);
        register_method("set_lyrics", &Tagging::set_lyrics);
        register_method("get_lyrics", &Tagging::get_lyrics);
        register_method("get_cover_description", &Tagging::get_cover_description);
        register_method("set_cover_description", &Tagging::set_cover_description);
        register_method("get_embedded_cover_count", &Tagging::get_embedded_cover_count);
    }

    void _init()
    {
    }

    enum {
        ARTIST,
        TITLE,
        ALBUM,
        GENRE,
        COMMENT,
        YEAR,
        TRACK
    };

    godot::PoolStringArray get_multiple_tags(godot::String path, godot::PoolIntArray tagmap) {
        return get_tag_data(path, tagmap);
    }

    godot::PoolByteArray copy_covers(godot::PoolStringArray src_paths, godot::PoolStringArray dst_paths) {
        //TagLib::ByteVector TempByteData;
        godot::PoolByteArray return_values;
        for (size_t i = 0; i < src_paths.size(); i++) {
            return_values.push_back(
                copy_cover(
                    src_paths[static_cast<unsigned int>(i)],
                    dst_paths[i]
                )
            );
        }
        return return_values;
    }

    godot::String get_single_tag(godot::String path, unsigned int tagflag) {
        godot::PoolIntArray x;
        x.push_back(tagflag);
        godot::PoolStringArray data = get_tag_data(path, x);
        if (data.size() == 0) { return {}; }
        return data[0];
    }

    void set_tag(unsigned int flag, godot::String tag_data, godot::String path) {
        TagLib::String npath = gd_2_taglib_string(path);
        TagLib::String ntagdata = gd_2_taglib_string(tag_data);
        TagLib::FileRef f(gd_string_to_filename(path));
        if (f.isNull()) {
            return;
        }

        switch (flag) {
        case ARTIST:
            f.tag()->setArtist(ntagdata);
            break;
        case TITLE:
            f.tag()->setTitle(ntagdata);
            break;
        case ALBUM:
            f.tag()->setAlbum(ntagdata);
            break;
        case GENRE:
            f.tag()->setGenre(ntagdata);
            break;
        case COMMENT:
            f.tag()->setComment(ntagdata);
            break;
        case YEAR:
            f.tag()->setYear(std::stoi(ntagdata.toCString(true)));
            break;
        case TRACK:
            f.tag()->setTrack(std::stoi(ntagdata.toCString(true)));
            break;
        default:
            //Invalid Flag given -> no Tag was set
            break;
        }

        //saving all changes made to the file
        f.save();
    }

    godot::PoolStringArray get_tag_data(godot::String path, godot::PoolIntArray type) {
        // This function receives a Path as std:: String and a PoolInt Array
        // The type array hast the indexes of the switch statement
        // This is done to be able to return multiple tags at once
        TagLib::FileRef f(gd_string_to_filename(path));
        godot::PoolStringArray text_tags = godot::PoolStringArray();
        text_tags.resize(type.size());
        if (!f.isNull()) {
            TagLib::String tagdata;
            for (size_t i = 0; i < type.size(); i++) {
                switch (type[static_cast<unsigned int>(i)]) {
                case ARTIST:
                    tagdata = f.tag()->artist();
                    break;
                case TITLE:
                    tagdata = f.tag()->title();
                    break;
                case ALBUM:
                    tagdata = f.tag()->album();
                    break;
                case GENRE:
                    tagdata = f.tag()->genre();
                    break;
                case COMMENT:
                    tagdata = f.tag()->comment();
                    break;
                case YEAR:
                    tagdata = std::to_string(f.tag()->year());
                    break;
                case TRACK:
                    tagdata = std::to_string(f.tag()->track());
                    break;
                }
                text_tags.set(i, tagdata.toCString(true));
            }
        }
        return text_tags;
    }

    godot::String get_cover_description(godot::String path) {
        TagLib::String npath = gd_2_taglib_string(path);
        TagLib::String fileType = get_extension(npath);
        if (fileType == "MP3" || fileType == "WAV") {
            TagLib::MPEG::File mp3File(gd_string_to_filename(path));
            TagLib::ID3v2::Tag* mp3Tag;
            TagLib::ID3v2::FrameList listOfMp3Frames;
            TagLib::ID3v2::AttachedPictureFrame* pictureFrame;
            if (mp3File.isOpen()) {
                if (!mp3File.hasID3v2Tag()) {
                    return "";
                }
                mp3Tag = mp3File.ID3v2Tag();
                if (mp3Tag != NULL)
                {
                    listOfMp3Frames = mp3Tag->frameListMap()["APIC"];//look for picture frames only
                    if (listOfMp3Frames.isEmpty()) {
                        return "";
                    }
                    pictureFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(listOfMp3Frames[0]);
                    return (pictureFrame->description()).toCString(true);
                }
            }
        }
        else if (fileType == "OGG") {
            TagLib::Ogg::Vorbis::File oggFile(gd_string_to_filename(path));
            TagLib::Ogg::XiphComment* tag = oggFile.tag();
            TagLib::List<TagLib::FLAC::Picture*> Pictures = tag->pictureList();
            return (Pictures[0]->description()).toCString(true);
        }
        return "";
    }


    void set_cover_description(godot::String path, godot::String NewCoverDescription) {
        TagLib::String npath = gd_2_taglib_string(path);

        TagLib::String fileType = get_extension(npath);
        if (fileType == "MP3" || fileType == "WAV") {

        }
        else if (fileType == "OGG") {
            TagLib::Ogg::Vorbis::File oggFile(gd_string_to_filename(path));
            TagLib::Ogg::XiphComment* tag = oggFile.tag();
            TagLib::List<TagLib::FLAC::Picture*> Pictures = tag->pictureList();
            Pictures[0]->setDescription(
                gd_2_taglib_string(NewCoverDescription)
            );
            oggFile.save();
        }
    }

    bool copy_cover(godot::String src_path, godot::String dst_path, unsigned int cover_idx = 0) {
        //TagLib::ByteVector tag_data;
        //Converts Hex Encoded Data
        TagLib::String nsrc_path = gd_2_taglib_string(src_path);
        TagLib::String ndst_path = gd_2_taglib_string(dst_path);
        TagLib::String fileType = get_extension(nsrc_path);
        if (fileType == "MP3" || fileType == "WAV") {
            TagLib::MPEG::File mp3File(gd_string_to_filename(src_path));
            TagLib::ID3v2::Tag* mp3Tag;
            TagLib::ID3v2::FrameList listOfMp3Frames;
            TagLib::ID3v2::AttachedPictureFrame* pictureFrame;
            //Checks if the given Song Path COul be opened
            if (!mp3File.hasID3v2Tag()) { return false; }
            mp3Tag = mp3File.ID3v2Tag();
            if (mp3Tag != NULL)
            {
                listOfMp3Frames = mp3Tag->frameListMap()["APIC"];//look for picture frames only
                if (!listOfMp3Frames.isEmpty())
                {
                    pictureFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame*> (listOfMp3Frames[cover_idx]);
                    //Warning. format of picture assumed to be jpg. This may be false, for example it may be png.
                    FILE* fout = NULL;
                    fout = fopen(ndst_path.toCString(true), "wb");
                    if (fout != NULL) {
                        fwrite(pictureFrame->picture().data(), pictureFrame->picture().size(), 1, fout);
                        pictureFrame->picture().data();
                        fclose(fout);
                    }
                    else {
                        return false;
                    }
                }
                else {
                    return false;
                }

            }
            else {
                return false;
            }
        }
        else if (fileType == "OGG") {
            //Copying Cover from OGG Vorbis File
            TagLib::Ogg::Vorbis::File oggFile(gd_string_to_filename(src_path));
            TagLib::Ogg::XiphComment* tag = oggFile.tag();
            TagLib::List<TagLib::FLAC::Picture*> Pictures = tag->pictureList();
            if (Pictures.isEmpty()) {
                return false;
            }
            FILE* fout = NULL;
            fout = fopen(ndst_path.toCString(true), "wb");
            if (fout != NULL) {
                //Returns the first cover of this OGG file
                fwrite(Pictures[cover_idx]->data().data(), Pictures[cover_idx]->data().size(), 1, fout);
                fclose(fout);
            }
        }
        else if (fileType == "LAC") {
            TagLib::FLAC::File FLACFile(gd_string_to_filename(src_path));
            TagLib::List<TagLib::FLAC::Picture*> Pictures = FLACFile.pictureList();
            if (Pictures.isEmpty()) {
                return false;
            }
            FILE* fout = NULL;
            fout = fopen(ndst_path.toCString(true), "wb");
            if (fout != NULL) {
                //Returns the first cover of this OGG file
                fwrite(Pictures[cover_idx]->data().data(), Pictures[cover_idx]->data().size(), 1, fout);
                fclose(fout);
            }
        }
        return true;
    }


    bool add_cover(godot::String audiopath, godot::String img_path, godot::String g_mime_type) {
        // Appends the Image into the File without changing the others
        TagLib::String naudiopath = gd_2_taglib_string(audiopath);
        TagLib::String mime_type = gd_2_taglib_string(g_mime_type);
        TagLib::String fileType = naudiopath.substr(naudiopath.size() - 3).upper();
        ImageFile imageFile(gd_string_to_filename(img_path));

        godot::Godot::print("APPENDING COVER IN C++");
        godot::Godot::print(img_path);
        godot::Godot::print(audiopath);

        if (!imageFile.isOpen()) {
            godot::Godot::print("IMAGE COULD NOT BE OPENED");
            return false;
        }

        if (fileType == "M4A")
        {
            TagLib::MP4::CoverArt coverArt((TagLib::MP4::CoverArt::Format)0x0D, imageFile.data());
            TagLib::MP4::File audioFile(gd_string_to_filename(audiopath));
            TagLib::MP4::Tag* tag = audioFile.tag();
            TagLib::MP4::ItemListMap itemsListMap = tag->itemListMap();

            // create cover art list
            TagLib::MP4::CoverArtList coverArtList;

            // append instance
            coverArtList.append(coverArt);

            // convert to item
            TagLib::MP4::Item coverItem(coverArtList);

            // add item to map
            itemsListMap.insert("covr", coverItem);

            tag->save();
            audioFile.save();
        }
        else if (fileType == "MP3" || fileType == "WAV")
        {
            godot::Godot::print("MP3 OR WAVE");
            TagLib::MPEG::File audioFile(gd_string_to_filename(audiopath));
            TagLib::ID3v2::Tag* tag = audioFile.ID3v2Tag(true);
            TagLib::ID3v2::FrameList AttachedCovers = tag->frameListMap()["APIC"];
            tag->addFrame(create_APIC_frame(imageFile.data(), mime_type));
            audioFile.save();
        }
        else if (fileType == "OGG") {
            OGG_VORBIS().add_cover(audiopath, img_path, g_mime_type);
        }
        else if (fileType == "LAC") {
            TagLib::FLAC::File FLACFile(gd_string_to_filename(audiopath));
            FLACFile.addPicture(create_flac_picture(imageFile.data(), mime_type));
            FLACFile.save();
        }
        else
        {
            godot::Godot::print("Invalid File Extension");
            return false;
        }
        return true;
    }


    bool remove_cover(godot::String audiopath, unsigned int cover_idx) {
        // Appends the Image into the File without changing the others
        TagLib::String naudiopath = gd_2_taglib_string(audiopath);
        TagLib::String dst_file = gd_2_taglib_string(audiopath);
        TagLib::String fileType = naudiopath.substr(naudiopath.size() - 3).upper();

        if (fileType == "M4A")
        {
            // read the mp4 file
            TagLib::MP4::File audioFile(gd_string_to_filename(audiopath));

            // get the tag ptr
            TagLib::MP4::Tag* tag = audioFile.tag();

            // get the items map
            TagLib::MP4::ItemListMap itemsListMap = tag->itemListMap();

            // create cover art list
            TagLib::MP4::CoverArtList coverArtList;

            // append instance
            coverArtList =  itemsListMap.value("covr").toCoverArtList();

            // convert to item
            TagLib::MP4::Item coverItem(coverArtList);

            // add item to map
            itemsListMap.insert("covr", coverItem);

            tag->save();
            audioFile.save();
        }
        else if (fileType == "MP3" || fileType == "WAV")
        {
            TagLib::MPEG::File audioFile(gd_string_to_filename(audiopath));
            TagLib::ID3v2::Tag* tag = audioFile.ID3v2Tag(true);
            tag->removeFrame(tag->frameListMap()["APIC"][cover_idx], true);
            audioFile.save();
        }
        else if (fileType == "OGG") {
            TagLib::Ogg::Vorbis::File oggFile(gd_string_to_filename(audiopath));
            TagLib::Ogg::XiphComment* tag = oggFile.tag();
            
            // removing a specific index of a an embedded picture within the file
            TagLib::List<TagLib::FLAC::Picture*> pictures = tag->pictureList();  // .begin() and .end() iterators have to be called by the same list
            unsigned int counter = 0;
            for (TagLib::List<TagLib::FLAC::Picture*>::ConstIterator it = pictures.begin(); it != pictures.end(); it++) {
                if (counter == cover_idx) {
                    tag->removePicture(*it);
                    break;
                }
                counter += 1;
            }
            oggFile.save();
        }
        else if (fileType == "LAC") {
            TagLib::FLAC::File FLACFile(gd_string_to_filename(audiopath));
            FLACFile.removePicture(FLACFile.pictureList()[cover_idx], true);
            FLACFile.save();
        }
        else
        {
            godot::Godot::print("Invalid File Extension");
            return false;
        }
        return true;
    }


    bool remove_all_covers(godot::String audiopath) {
        // Appends the Image into the File without changing the others
        TagLib::String naudiopath = gd_2_taglib_string(audiopath);
        TagLib::String fileType = naudiopath.substr(naudiopath.size() - 3).upper();

        if (fileType == "M4A")
        {
            // read the mp4 file
            TagLib::MP4::File audioFile(gd_string_to_filename(audiopath));

            // get the tag ptr
            TagLib::MP4::Tag* tag = audioFile.tag();

            // get the items map
            TagLib::MP4::ItemListMap itemsListMap = tag->itemListMap();

            // create cover art list
            TagLib::MP4::CoverArtList coverArtList;

            // remove all
            tag->removeItem("covr");
         

            tag->save();
            audioFile.save();
        }
        else if (fileType == "MP3" || fileType == "WAV")
        {
            TagLib::MPEG::File audioFile(gd_string_to_filename(audiopath));
            TagLib::ID3v2::Tag* tag = audioFile.ID3v2Tag(true);
            tag->removeFrames("APIC");
            audioFile.save();
        }
        else if (fileType == "OGG") {
            TagLib::Ogg::Vorbis::File oggFile(gd_string_to_filename(audiopath));
            TagLib::Ogg::XiphComment* tag = oggFile.tag();
            tag->removeAllPictures();
            oggFile.save();
        }
        else if (fileType == "LAC") {
            TagLib::FLAC::File FLACFile(gd_string_to_filename(audiopath));
            FLACFile.removePictures();
            FLACFile.save();
        }
        else
        {
            godot::Godot::print("Invalid File Extension");
            return false;
        }
        return true;
    }


    TagLib::ID3v2::AttachedPictureFrame* create_APIC_frame(TagLib::ByteVector data, TagLib::String mime_type) {
        TagLib::ID3v2::AttachedPictureFrame* new_frame = new TagLib::ID3v2::AttachedPictureFrame;
        new_frame->setMimeType(mime_type);
        new_frame->setPicture(data);
        return new_frame;
    }

    TagLib::FLAC::Picture* create_FLAC_picture(TagLib::ByteVector image_data, TagLib::String mime_type) {
        TagLib::FLAC::Picture * new_picture = new TagLib::FLAC::Picture;
        new_picture->setData(image_data);
        new_picture->setMimeType(mime_type);
        return new_picture;
    }


    godot::PoolByteArray get_embedded_cover(godot::String src_path, unsigned int cover_idx) {
        TagLib::String fileType = get_extension(src_path.alloc_c_string());
        if (fileType == "MP3" || fileType == "WAV") {
            return MPEG().get_cover(src_path, cover_idx);
        }
        else if (fileType == "OGG") {
            return OGG_VORBIS().get_cover(src_path, cover_idx);
        }
        else if (fileType == "LAC") {
            return FLAC().get_cover(src_path, cover_idx);
        }
        else if (fileType == "M4A" || fileType == "MP4") {
            return MP4().get_cover(src_path, cover_idx);
        }
        return godot::PoolByteArray();
    }


    godot::Array get_embedded_covers(godot::String src_path) {
        // Returns the Data of every Cover embededded in the File given
        // As godot::Array where each index is a godot::PoolByteArray
        TagLib::String fileType = get_extension(src_path.alloc_c_string());
        if (fileType == "MP3" || fileType == "WAV") {
            return MPEG().get_covers(src_path);
        }
        else if (fileType == "OGG") {
            return OGG_VORBIS().get_covers(src_path);
        }
        else if (fileType == "LAC") {
            return FLAC().get_covers(src_path);
        }
        else if (fileType == "M4A" || fileType == "MP4") {
            return MP4().get_covers(src_path);
        }
        return godot::Array();
    }


    unsigned int get_embedded_cover_count(godot::String src_path) {
        TagLib::String fileType = get_extension(src_path.alloc_c_string());
        if (fileType == "MP3" || fileType == "WAV") {
            return MPEG().get_embedded_cover_count(src_path);
        }
        else if (fileType == "OGG") {
            return OGG_VORBIS().get_embedded_cover_count(src_path);
        }
        else if (fileType == "LAC") {
            return FLAC().get_embedded_cover_count(src_path);
        }
        else if (fileType == "M4A" || fileType == "MP4") {
            return MP4().get_embedded_cover_count(src_path);
        }
        return 0;
    }


    void set_lyrics(godot::String file_path, godot::PoolStringArray Verses, godot::PoolIntArray TimeStamps, bool IsSynchronized) {
        TagLib::String npath = gd_2_taglib_string(file_path);
        std::string FrameTitle = "";
        if (IsSynchronized) {
            //[#sec4.10 Synchronized lyric/text]
            FrameTitle = "SYLT";
        }
        else {
            //[#sec4.9 Unsychronized lyric/text transcription]
            FrameTitle = "USLT";
        }

        TagLib::MPEG::File mp3File(gd_string_to_filename(file_path));

        TagLib::ID3v2::Tag* mp3Tag;

        mp3Tag = mp3File.ID3v2Tag(true);

        TagLib::ID3v2::FrameList listOfMp3Frames;
        if (!mp3File.isOpen()) {
            return;
        }

        listOfMp3Frames = mp3Tag->frameListMap()[FrameTitle.c_str()];
        for (size_t i = 0; i < listOfMp3Frames.size(); i++) {
            mp3Tag->removeFrame(listOfMp3Frames[i]);
        }


        if (IsSynchronized) {

            TagLib::ID3v2::SynchronizedLyricsFrame* NewSynchedLyricsFrame = new TagLib::ID3v2::SynchronizedLyricsFrame;
            TagLib::List< TagLib::ID3v2::SynchronizedLyricsFrame::SynchedText > SynchedText;



            for (size_t i = 0; i < Verses.size(); i++) {
                TagLib::ID3v2::SynchronizedLyricsFrame::SynchedText SynchedVerse = {
                    static_cast<unsigned int> (TimeStamps[i]),
                    Verses[i].alloc_c_string(),
                };
                SynchedText.append(SynchedVerse);
            }

            NewSynchedLyricsFrame->setTimestampFormat(NewSynchedLyricsFrame->AbsoluteMilliseconds);
            NewSynchedLyricsFrame->setSynchedText(SynchedText);

            mp3Tag->addFrame(NewSynchedLyricsFrame);
        }
        else {

            TagLib::ID3v2::UnsynchronizedLyricsFrame* NewUnsynchedLyricsFrame = new TagLib::ID3v2::UnsynchronizedLyricsFrame();
            std::string Lyrics = "";
            for (size_t i = 0; i < Verses.size(); i++) {
                Lyrics.append(Verses[i].alloc_c_string());
                Lyrics.append("\n");
            }
            NewUnsynchedLyricsFrame->setText(Lyrics);
            mp3Tag->addFrame(NewUnsynchedLyricsFrame);
        }
        mp3File.save();
    }


    godot::Array get_lyrics(godot::String src_path) {
        //Retrieving the Lyrics from an Audiofile
        //Synchronized Lyrics will be preferred to Unsynchronized ones
        TagLib::String path = gd_2_taglib_string(src_path);
        TagLib::MPEG::File file(gd_string_to_filename(src_path));
        if (!file.isOpen()) { return godot::Array(); }
        if (!file.hasID3v2Tag()) { return godot::Array(); }

        TagLib::ID3v2::Tag* ID3Tag = file.ID3v2Tag();
        TagLib::ID3v2::FrameList synched_frames = ID3Tag->frameListMap()["SYLT"];
        if (!synched_frames.isEmpty()) {
            TagLib::ID3v2::SynchronizedLyricsFrame* SynchedFrame = static_cast<TagLib::ID3v2::SynchronizedLyricsFrame*> (synched_frames[0]);
            godot::PoolStringArray synched_text;
            godot::PoolRealArray synched_timestamps;
            TagLib::List< TagLib::ID3v2::SynchronizedLyricsFrame::SynchedText > x = SynchedFrame->synchedText();
            //SynchedFrame->timestampFormat();


            for (size_t i = 0; i < x.size(); i++) {
                synched_text.push_back(x[i].text.toCString(true));
                TagLib::ID3v2::SynchronizedLyricsFrame* SynchedFrame = static_cast<TagLib::ID3v2::SynchronizedLyricsFrame*> (synched_frames[0]);
                switch (SynchedFrame->timestampFormat()) {
                case 2:             //Absolute Milliseconds
                    synched_timestamps.push_back(static_cast<float>(x[i].time) / 1000.0);
                    break;
                case 1:             //Absolute MPEG Frames
                {
                    //Converting the Absolute MPEG Frames to Seconds
                    unsigned int sample_rate = file.audioProperties()->sampleRate();
                    unsigned int samples_per_frame = 0;
                    unsigned int abs_mpeg_frames = x[i].time;
                    if (sample_rate >= 16000 && sample_rate <= 24000) {
                        samples_per_frame = 576;
                    }
                    else if (sample_rate >= 32000 && sample_rate <= 48000) {
                        samples_per_frame = 1152;
                    }
                    synched_timestamps.push_back(1000.0 / (file.audioProperties()->sampleRate()) * samples_per_frame * abs_mpeg_frames / 1000.0);
                    break;
                }
                case 0:             //Unknown
                    //Trying to interpret them as seconds, beacause it's the most common format
                    synched_timestamps.push_back(static_cast<float>(x[i].time) / 1000.0);
                    break;
                }
            }

            godot::Array synched_lyrics;

            synched_lyrics.push_back(synched_text);
            synched_lyrics.push_back(synched_timestamps);

            //Array -> [Lyrics, TimeStamps]
            return synched_lyrics;
            //If there is a Synchrnozed Lyrics Frame
        }
        else {
            synched_frames = ID3Tag->frameListMap()["USLT"];
            if (!synched_frames.isEmpty()) {
                //if there is an unsynchronized Lyrics Frame
                TagLib::ID3v2::UnsynchronizedLyricsFrame* unsynched_frame = static_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*> (synched_frames[0]);
                godot::Array unsynched_text;
                unsynched_text.push_back(unsynched_frame->text().toCString(true));
                return unsynched_text;
            }
        }
        return godot::Array();
    }


    void set_song_popularity(godot::String file_path, int rating_out_of_10 = -1, int Counter = -1, godot::String Email = "") {
        TagLib::String path = gd_2_taglib_string(file_path);
        TagLib::String email = gd_2_taglib_string(Email).toCString(true);
        TagLib::String fmt = get_extension(path);
        
        if (fmt == "MP3" || fmt == "WAV") {
            
            TagLib::MPEG::File ID3file(gd_string_to_filename(file_path));
            TagLib::ID3v2::Tag* ID3Tag = ID3file.ID3v2Tag();
            
            if (ID3Tag == NULL) { return; }
            TagLib::ID3v2::FrameList RatingFrames = ID3Tag->frameListMap()["POPM"];
            TagLib::ID3v2::PopularimeterFrame* rating_frame = new TagLib::ID3v2::PopularimeterFrame();
            if (RatingFrames.size() > 0) {
                godot::Array frame_data =  get_song_popularity(file_path);
                ID3Tag->removeFrames("POPM");
                rating_frame->setRating(frame_data[0]);
                rating_frame->setCounter(frame_data[1]);
                rating_frame->setEmail(gd_2_taglib_string(frame_data[2]));
            }
              
            // filling in frame data
            if (email != "") { rating_frame->setEmail(email.toCString(true)); }
            if (rating_out_of_10 != -1) { rating_frame->setRating(rating_out_of_10); }
            if (Counter != -1) { rating_frame->setCounter(Counter); }
                
            ID3Tag->addFrame(static_cast<TagLib::ID3v2::Frame*>(rating_frame));
            ID3file.save();
            return;
        }
        else if (fmt == "OGG") {
            //Adding using custom fields -> non standarddized way
            TagLib::Ogg::Vorbis::File OggFile(gd_string_to_filename(file_path));
            TagLib::Ogg::XiphComment* OggTag = OggFile.tag();
            if (rating_out_of_10 != -1) { OggTag->addField("Rating", std::to_string(rating_out_of_10), true); }
            if (Counter != -1) { OggTag->addField("Counter", std::to_string(Counter), true); }
            if (Email != "") { OggTag->addField("Email", email.toCString(true), true); }
            OggFile.save();
        }
    }


    godot::Array get_song_popularity(godot::String file_path) {
        TagLib::String path = gd_2_taglib_string(file_path);
        godot::Array rating_data;
        rating_data.resize(3);

        std::string fmt = get_extension(path).toCString(true);
        if (fmt == "MP3" || fmt == "WAV") {
            // reading the rating from the frame Popularitymeter POPM if existing
            TagLib::MPEG::File ID3file(gd_string_to_filename(file_path));
            if (!ID3file.isOpen()) { return rating_data; }

            TagLib::ID3v2::Tag* ID3Tag = ID3file.ID3v2Tag();
            TagLib::ID3v2::FrameList RatingFrames = ID3Tag->frameListMap()["POPM"];
            if (RatingFrames.size() > 0) {
                TagLib::ID3v2::PopularimeterFrame* RatingFrame = static_cast<TagLib::ID3v2::PopularimeterFrame*>(RatingFrames[0]);
                rating_data[0] = RatingFrame->rating();
                rating_data[1] = RatingFrame->counter();
                rating_data[2] = RatingFrame->email().toCString(true);
            }
        }
        else if (fmt == "OGG") {
            // reading the rating from custom fields -> Rating, Counter, Email
            TagLib::Ogg::Vorbis::File ogg_file(gd_string_to_filename(file_path));
            if (!ogg_file.isOpen()) { return rating_data; }
            TagLib::Ogg::XiphComment* OggTag = ogg_file.tag();
            TagLib::PropertyMap fields = OggTag->properties();
            if (fields.contains("Rating")) {
                rating_data[0] = (fields["Rating"][0]).toCString(true);
            }
            if (fields.contains("Counter")) {
                rating_data[1] = (fields["Counter"][0]).toCString(true);
            }
            if (fields.contains("Email")) {
                rating_data[2] = (fields["Email"][0]).toCString(true);
            }
        }
        return rating_data;
    }
};

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options * o)
{
    godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options * o)
{
    godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void* handle)
{
    godot::Godot::nativescript_init(handle);
    godot::register_class<Tagging>();
    godot::register_class<AudioProperties>();
    godot::register_class<MPEG>();
    godot::register_class<MP4>();
    godot::register_class<FLAC>();
    godot::register_class<OGG_VORBIS>();
}
