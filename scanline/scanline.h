#ifndef SCANLINE_H
#define SCANLINE_H

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/polygon/polygon.hpp>

namespace gtl = boost::polygon;

using gtlCoord = int64_t;
using gtlPoint = gtl::point_data<gtlCoord>;

struct gtlPointHash {
    size_t operator()(const gtlPoint& pt) const {
        size_t h1 = std::hash<gtlCoord>{}(pt.x());
        size_t h2 = std::hash<gtlCoord>{}(pt.y());
        return h1 ^ (h2 << 32);
    }
};

using gtlSegment = gtl::segment_data<gtlCoord>;
using gtlRect = gtl::rectangle_data<gtlCoord>;

using gtlPolygonBase = gtl::polygon_data<gtlCoord>;
using gtlPolygon = gtl::polygon_with_holes_data<gtlCoord>;

inline void
BufferSegments(const std::vector<gtlSegment>& segs,
              std::vector<gtlSegment>& bufSegs /*out*/)
{
    bufSegs.clear();
    for (const auto& seg : segs) {
        bufSegs.push_back(seg);
    }
}


enum class ScanDir {
    HOR, // horizontal scan, line is vertical
    VER, // vertical scan, line is horizontal
};

using RoomType = int8_t;

struct SegLoc : public gtlSegment
{
    SegLoc (const gtlPoint& pt1, const gtlPoint& pt2) :
        gtlSegment(pt1, pt2)
    {
        if (pt1 == pt2) {
            throw std::invalid_argument("Segment points cannot be the same");
        }
        SortByYThenX();

        if (!IsHor()) {
            assert(high().x() ==
                   (GetXOfYmin() + SlopRecipTimesDeltaY(GetYmax() - GetYmin())));
        }
    }

    SegLoc (gtlCoord x1, gtlCoord y1, gtlCoord x2, gtlCoord y2) :
        SegLoc(gtlPoint(x1, y1), gtlPoint(x2, y2))
    {}

    explicit SegLoc (const gtlSegment& seg) :
        SegLoc(seg.low(), seg.high())
    {}

    // get point location
    gtlPoint GetLoPt() const noexcept {
        return low();
    }

    gtlPoint GetHiPt() const noexcept {
        return high();
    }

    gtlPoint GetCenterPt() const noexcept {
        return gtlPoint((low().x() + (high().x() - low().x()) / 2),
                        (low().y() + (high().y() - low().y()) / 2));
    }

    bool HasEndPt (const gtlPoint& pt) const noexcept {
        return pt == low() || pt == high();
    }
    
    //--------------------------------
    bool IsHor() const noexcept {
        return low().y() == high().y();
    }
    
    bool IsVert() const noexcept {
        return low().x() == high().x();
    }

    gtlCoord GetXLength () const noexcept {
        return std::abs(high().x() - low().x());
    }

    gtlCoord GetYLength () const noexcept {
        return std::abs(high().y() - low().y());
    }

#if 0
    gtlCoord GetXmax() const noexcept {
        return std::max(low().x(), high().x());
    }
    
    gtlCoord GetXmin() const noexcept {
        return std::min(low().x(), high().x());
    }
#endif
    
    gtlCoord GetYmax() const noexcept {
        return high().y();
    }
    
    gtlCoord GetYmin() const noexcept {
        return low().y();
    }
    
    gtlCoord GetXOfYmin() const noexcept {
        return low().x();
    }
    
    // slope related functions
    double CalSlopeReciprocal() const noexcept {
        if (IsHor()) {
            assert(0);
            return 1.0e9;
        }
        else if (IsVert()) {
            return 0.0;
        }
        else {
            return (double)(high().x() - low().x()) / (high().y() - low().y());
        }
    }

    gtlCoord SlopRecipTimesDeltaY (gtlCoord deltaY) const noexcept {
        return ((high().x() - low().x()) * deltaY) / (high().y() - low().y());
    }
    
    gtlCoord DeltaX(gtlCoord deltaY) const;
    gtlCoord DeltaY(gtlCoord deltaX) const;
    
    std::string ToStr() const;

private:
    void SwapPts () noexcept {
        auto lo = low();
        auto hi = high();
        low(hi);
        high(lo);
    }

    void SortByYThenX () noexcept {
        if (high().y() < low().y()) {
            SwapPts();
        }
        else if (high().y() == low().y() &&
                 high().x() < low().x()) {
            SwapPts();
        }
    }
};

#if 0
template <ScanDir scanDir>
struct SStat : public SegLoc
{
    // from low to high
    enum class EdgeSide {
        IN, OUT,
        NONE
    };

    SStat (const gtlPoint& p1, const gtlPoint& p2,
           EdgeSide edgeSide) :
        SegLoc(p1, p2),
        _edgeSide(edgeSide)
    {
        if constexpr (scanDir == ScanDir::VER)
        {
            _loc = GetXOfYmin();
        }
        else
        {
            _loc = GetYOfXmin();
        }
    }

    bool IsEdgeIn () const noexcept {
        return _edgeSide == EdgeSide::IN;
    }

    bool IsEdgeOut () const noexcept {
        return _edgeSide == EdgeSide::OUT;
    }

    bool IsParallel () const noexcept {
        if constexpr (scanDir == ScanDir::VER) {
            return IsHor();
        }
        else {
            return IsVert();
        }
    }

    double SlopeReciprocal () const noexcept {
        if constexpr (scanDir == ScanDir::VER) {
            if (IsHor()) {
                if (_loc == high().x()) {
                    return -1.0e9;
                }
                else {
                    return 1.0e9;
                }
            }
            else {
                return (double)(high().x() - low().x()) / (high().y() - low().y());
            }
        }
        else {
            if (IsVert()) {
                if (_loc == high().y()) {
                    return -1.0e9;
                }
                else {
                    return 1.0e9;
                }
            }
            else {
                return (double)(high().y() - low().y()) / (high().x() - low().x());
            }
        }
    }

    int64_t GetLoc () const noexcept {
        return _loc;
    }

    int64_t GetPosMin () const noexcept {
        if constexpr (scanDir == ScanDir::VER) {
            return GetYmin();
        }
        else {
            return GetXmin();
        }
    }
    bool IsStartPos (gtlCoord pos) const noexcept {
        if constexpr (scanDir == ScanDir::VER) {
            return pos == low().y();
        }
        else {
            return pos == std::min(low().x(), high().x());
        }
    }

    bool IsEndPos (gtlCoord pos) const noexcept {
        if constexpr (scanDir == ScanDir::VER) {
            return pos == high().y();
        }
        else {
            return pos == std::max(low().x(), high().x());
        }
    }

    bool IsTerminalPos (gtlCoord pos) const noexcept {
        return IsStartPos(pos) || IsEndPos(pos);
    }

    gtlCoord
    UpdateLoc (gtlCoord pos) const noexcept
    {
        if constexpr (scanDir == ScanDir::VER) {
            return GetXOfYmin() + DeltaX(pos - GetYmin());
        }
        else {
            return GetYOfXmin() + DeltaY(pos - GetXmin());
        }
    }

    // loc is current location in another direction of the scan line
    int64_t         _loc;
    EdgeSide        _edgeSide;
};

#if 0
// scan from low to high in Y direction
class SegmentsIntersect
{
    struct ScanStat {
        ScanStat (const SegLoc& segLoc) :
            segLoc(segLoc),
            x(segLoc.GetXOfYmin())
        {
        }
        SegLoc      segLoc;
        int64_t     x;      // current x position
        ScanStat*   next{nullptr};
    };

public:
    SegmentsIntersect (const std::vector<gtlSegment>& segs);

private:
    void BuildEdgeTbl (const std::vector<gtlSegment>& segs);

    std::vector<SegLoc>         _segs;

    std::vector<int64_t>        _yPosArr;
    int64_t                     _yPosEnd; // end of yPos, yPos[yPosEnd - 1] is the highest yPos

}; // end of class Scanner

void
SegmentsIntersect::BuildEdgeTbl (const std::vector<gtlSegment>& segs)
{
    size_t segsSize = segs.size();
    _segs.resize(segsSize);
    for (size_t i = 0; i < segsSize; ++i) {
        _segs[i] = SegLoc(segs[i].low(), segs[i].high());
    }

    std::sort(_segs.begin(), _segs.end(),
              [](const SegLoc& a, const SegLoc& b) {
                  if (a.GetPt1().y() != b.GetPt1().y()) {
                      return a.GetPt1().y() < b.GetPt1().y();
                  }
                  return a.GetPt1().x() < b.GetPt1().x();
              });

    // collect all yPos, sort and unique
    _yPosArr.resize(segsSize * 2);
    for (size_t i = 0; i < segsSize; ++i) {
        _yPosArr[i * 2] = _segs[i].GetPt1().y();
        _yPosArr[i * 2 + 1] = _segs[i].GetPt2().y();
    }
    std::sort(_yPosArr.begin(), _yPosArr.end());
    _yPosEnd = std::unique(_yPosArr.begin(), _yPosArr.end()) - _yPosArr.begin();
}
#endif
class PolygonEdgeQuickSearch
{
public:
    PolygonEdgeQuickSearch(const gtlPolygonBase& poly);

private:
    gtlPolygonBase _poly;
};
#endif

#endif // SCANLINE_H
