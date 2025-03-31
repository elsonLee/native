#ifndef SCANLINE_H
#define SCANLINE_H

#include <cassert>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <boost/polygon/polygon.hpp>

#define DEBUG 0

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
inline constexpr RoomType NO_ROOM_TYPE = -1;

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

    bool IsVer() const noexcept {
        return low().x() == high().x();
    }

    gtlCoord GetXLength () const noexcept {
        return std::abs(high().x() - low().x());
    }

    gtlCoord GetYLength () const noexcept {
        return std::abs(high().y() - low().y());
    }

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
        else if (IsVer()) {
            return 0.0;
        }
        else {
            return (double)(high().x() - low().x()) / (high().y() - low().y());
        }
    }

    gtlCoord SlopRecipTimesDeltaY (gtlCoord deltaY) const noexcept {
        return ((high().x() - low().x()) * deltaY) / (high().y() - low().y());
    }

    void SetLBType (RoomType type) noexcept {
        _lbType = type;
    }
    RoomType GetLBType () const noexcept {
        return _lbType;
    }

    void SetRTType (RoomType type) noexcept {
        _rtType = type;
    }
    RoomType GetRTType () const noexcept {
        return _rtType;
    }

    std::string ToStr () const noexcept {
        return "{" +
               std::to_string(low().x()) + ", " + std::to_string(low().y()) +
               "} -> {" +
               std::to_string(high().x()) + ", " + std::to_string(high().y()) +
               "}";
    }

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

    RoomType _lbType{NO_ROOM_TYPE};
    RoomType _rtType{NO_ROOM_TYPE};
};

inline bool
operator== (const SegLoc& lhs, const SegLoc& rhs) noexcept
{
    return lhs.GetLoPt() == rhs.GetLoPt() &&
           lhs.GetHiPt() == rhs.GetHiPt();
}

struct SegLocHash {
    size_t operator()(const SegLoc& segLoc) const noexcept {
        size_t h1 = std::hash<gtlCoord>{}(segLoc.GetLoPt().x());
        size_t h2 = std::hash<gtlCoord>{}(segLoc.GetLoPt().y());
        size_t h3 = std::hash<gtlCoord>{}(segLoc.GetHiPt().x());
        size_t h4 = std::hash<gtlCoord>{}(segLoc.GetHiPt().y());
        return h1 ^ (h2 << 32) ^ (h3 << 3) ^ (h4 << 29);
    }
};

class Room;

class Seg : public SegLoc
{
    friend class Room;
    template <typename T>
    friend class RMGraphBuilder;

public:
    Seg (gtlCoord pt1X, gtlCoord pt1Y,
         gtlCoord pt2X, gtlCoord pt2Y) :
         SegLoc(pt1X, pt1Y, pt2X, pt2Y)
    {}

    Seg (const gtlPoint& pt1, const gtlPoint& pt2) :
        SegLoc(pt1, pt2)
    {}

    // left & bottom room
    bool IsLRoom (const Room* room) {
        SetLRoom(_lbRoom);
        return _lbRoom.get() == room;
    }
    void SetLRoom (std::shared_ptr<Room> room);

    std::shared_ptr<Room>
    LRoom () {
        SetLRoom(_lbRoom);
        return _lbRoom;
    }

    bool IsRRoom (const Room* room) {
        SetRRoom(_rtRoom);
        return _rtRoom.get() == room;
    }
    void SetRRoom (std::shared_ptr<Room> room);

    std::shared_ptr<Room>
    RRoom () {
        SetRRoom(_rtRoom);
        return _rtRoom;
    }

private:
    void
    SetTypeFrLoc (RoomType lbType, RoomType rtType) noexcept
    {
        if (lbType != NO_ROOM_TYPE) {
            SetLBType(lbType);
        }
        if (rtType != NO_ROOM_TYPE) {
            SetRTType(rtType);
        }
    }

private:
    mutable std::shared_ptr<Room>   _lbRoom{nullptr};
    mutable std::shared_ptr<Room>   _rtRoom{nullptr};
};

class Room : public std::enable_shared_from_this<Room>
{
    friend class Seg;

public:
    explicit Room (uint64_t id) :
        _id(id),
        _parent(nullptr)
    {}
    ~Room () {

    }

    uint64_t GetId () const noexcept {
        return _id;
    }

    RoomType GetType () const noexcept {
        return _type;
    }

    Room*
    GetOuterRoom (const SegLoc& segLoc) const noexcept {
        assert(this);
        auto seg = GetSegByLoc(segLoc);
        if (seg) {
            if (seg->LRoom() == shared_from_this()) {
                return seg->RRoom().get();
            } else if (seg->RRoom() == shared_from_this()) {
                return seg->LRoom().get();
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    /*
     * segs are sorted in CW order
     */
    const std::vector<Seg*>&
    GetSegList () const noexcept {
        return _segs;
    }

    Seg*
    GetSegByLoc (const SegLoc& segLoc) const noexcept {
        for (auto seg : _segs) {
            assert(seg);
            if (*seg == segLoc) {
                return seg;
            }
        }
        return nullptr;
    }

    bool HasSeg (const Seg* seg) const noexcept {
        if (!seg) {
            return false;
        }
        for (auto s : _segs) {
            assert(s);
            if (s == seg) {
                return true;
            }
        }
        return false;
    }

    gtlPolygon GetPolygon () const noexcept {
        return _pl;
    }

private:

    void SetId (uint64_t id) noexcept {
        _id = id;
    }

    void SetType (RoomType type) noexcept
    {
        if (type == NO_ROOM_TYPE ||
            type == _type)
        {
            return;
        }
        if (_type == NO_ROOM_TYPE) {
            _type = type;
        } else {
            printf("ERROR: set Room-%lu type from %d to %d!\n",
                   GetId(), _type, type);
        }
    }

    bool HasParent () const noexcept {
        return _parent? true : false;
    }

    std::shared_ptr<Room>
    Get () {
        auto sp = shared_from_this();
        assert(sp);
        while (sp->_parent) {
            sp = sp->_parent;
            _parent = sp;
        }
        return sp;
    }

    bool
    AddSeg (Seg* seg) noexcept
    {
        assert(seg);
        // TODO: optimize
        if (HasSeg(seg)) {
#if DEBUG
            printf("seg %s already in room %lu\n", seg->ToStr().c_str(), _id);
#endif
            return false;
        }
        _segs.push_back(seg);

#if DEBUG
        printf("seg %s added to room %lu\n", seg->ToStr().c_str(), _id);
        printf("\tRoom-%lu, added seg: %s, LRoom-%lu, RRoom-%lu\n",
               _id, seg->ToStr().c_str(),
               seg->LRoom()->GetId(),
               seg->RRoom()->GetId());
#endif
        return true;
    }

    int
    AddSegs (const std::vector<Seg*>& segs) noexcept
    {
        int count = 0;
        for (auto seg : segs) {
            if (AddSeg(seg)) {
                count++;
            }
        }
        return count;
    }

    bool
    DetachSeg (const Seg* seg)
    {
        if (!seg) {
            return false;
        }

        auto iter = std::find(_segs.begin(), _segs.end(), seg);
        if (iter != _segs.end()) {
            _segs.erase(iter);
            if (seg->_lbRoom == shared_from_this()) {
                seg->_lbRoom = nullptr;
            } else {
                assert(seg->_rtRoom == shared_from_this());
                seg->_rtRoom = nullptr;
            }
            return true;
        } else {
            return false;
        }
    }

private:
    uint64_t                _id;
    RoomType                _type{NO_ROOM_TYPE};
    std::shared_ptr<Room>   _parent{nullptr}; // union set

    std::vector<Seg*>       _segs;

    gtlPolygon              _pl;
};

template <typename RoomT = Room,
          typename SegT = Seg>
class RMGraph
{
    friend class Room;
    friend class Seg;
    template <typename T>
    friend class RMGraphBuilder;

public:
    RMGraph () = default;
    RMGraph (const RMGraph&) = delete;
    RMGraph& operator= (const RMGraph&) = delete;

    RMGraph (RMGraph&&) = default;
    RMGraph& operator= (RMGraph&&) = default;

    ~RMGraph ();

protected:
    std::shared_ptr<RoomT>
    AllocRoom () noexcept;

    // merge room2 to room1, room2 will be deleted
    std::shared_ptr<RoomT>
    MergeRooms (std::shared_ptr<RoomT> room1,
                std::shared_ptr<RoomT> room2) noexcept;

    // segment related functions
    using SegMap_t = typename std::unordered_map<SegLoc, SegT*, SegLocHash>;

    bool HasSeg (const SegLoc& segLoc) const noexcept {
        return _segMap.find(segLoc) != _segMap.end();
    }

    Seg*
    GetSeg (const SegLoc& segLoc) const noexcept {
        auto iter = _segMap.find(segLoc);
        if (iter == _segMap.end()) {
            return nullptr;
        }
        return iter->second;
    }

    Seg*
    GocSeg (const SegLoc& segLoc,
            bool* isNew = nullptr /*out*/)
    {
        auto iter = _segMap.find(segLoc);
        if (iter == _segMap.end()) {
            auto nSeg = new SegT(segLoc.GetLoPt(), segLoc.GetHiPt());
            _segMap[segLoc] = nSeg;
            if (isNew) {
                *isNew = true;
            }
            return nSeg;
        } else {
            assert(*_segMap[segLoc] == segLoc);
            if (isNew) {
                *isNew = false;
            }
            auto seg = iter->second;
            assert(seg);
            return seg;
        }
    }

    typename SegMap_t::iterator
    DelSeg (typename SegMap_t::iterator iter)
    {
        auto seg = iter->second;
        assert(seg);
        auto lbRoom = seg->LRoom();
        if (lbRoom) {
            lbRoom->DetachSeg(seg);
        }
        auto rtRoom = seg->RRoom();
        if (rtRoom) {
            rtRoom->DetachSeg(seg);
        }
        delete seg;
        return _segMap.erase(iter);
    }

    void
    SetDftRoomType (RoomType type) noexcept {
        _dftRoomType = type;
    }
    RoomType
    GetDftRoomType () const noexcept {
        return _dftRoomType;
    }

protected:
    RoomType    _dftRoomType{NO_ROOM_TYPE}; // default room type
    uint64_t    _nextRoomId{0};

    mutable std::vector<std::weak_ptr<RoomT>> _roomList;
    mutable SegMap_t                          _segMap;
};

template <typename Graph = RMGraph<Room, Seg>>
class RMGraphBuilder
{
private:
    struct SegStat
    {
        gtlCoord UpdateX (gtlCoord y) noexcept {
            x = seg->GetXOfYmin() + seg->SlopRecipTimesDeltaY(y - seg->GetYmin());
            return x;
        }

        gtlCoord    x;  // current x
        Seg*        seg;
    };

public:
    using EdgeTbl = std::map<gtlCoord, std::vector<SegStat>, std::less<gtlCoord>>;
    using HorSegTbl = std::map<gtlCoord, std::vector<SegStat>, std::less<gtlCoord>>;

    template <typename... Args>
    RMGraphBuilder (Args&&... args) :
        _graph(std::forward<Args>(args)...)
    {}

    RoomType GetDftRoomType () const noexcept {
        return _dftRoomType;
    }

    RMGraphBuilder&
    SetDftRoomType (RoomType type) noexcept {
        _dftRoomType = type;
        _graph.SetDftRoomType(type);
        return *this;
    }

    RMGraphBuilder&
    AddSegs (const std::vector<SegLoc>& segLocs) noexcept
    {
        _segLocs.insert(_segLocs.end(),
                        segLocs.begin(), segLocs.end());
        return *this;
    }

    RMGraphBuilder&
    AddRegions (const std::vector<gtlPolygon>& pls, RoomType type = -1) noexcept;

    Graph BuildGraph ();

private:
    std::vector<Seg*>
    SplitToNonOverlapSegs (const std::vector<SegLoc>& segLocs);

private:
    std::vector<SegLoc>     _segLocs;
    RoomType                _dftRoomType{NO_ROOM_TYPE}; // default room type
    Graph                   _graph;

private:
    void BuildEdgeTbl ();

    EdgeTbl                 _ET;
    std::vector<SegStat>    _AEL; // active edge list

    // tmp
    HorSegTbl               _horSegTbl;
};

namespace boost::polygon {
    template <>
    struct segment_traits<SegLoc> {
        typedef gtlCoord coordinate_type;
        typedef gtlPoint point_type;

        static inline point_type get(const SegLoc& segment, direction_1d dir) {
            return dir.to_int() ? segment.high() : segment.low();
        }

        static inline void set(SegLoc& segment, direction_1d dir, const point_type& point) {
            if (dir.to_int())
                segment.high(point);
            else
                segment.low(point);
        }
    };

    template <>
    struct geometry_concept<SegLoc> {
        typedef segment_concept type;
    };

} // namespace boost::polygon

#endif // SCANLINE_H
