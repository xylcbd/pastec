#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

struct SearchResult
{
    SearchResult(float f_weight, unsigned i_imageId)
        : f_weight(f_weight), i_imageId(i_imageId)
    {}

    bool operator< (const SearchResult &res) const
    {
        return f_weight < res.f_weight;
    }

    float f_weight;
    unsigned i_imageId;
};

#endif // SEARCHRESULT_H
