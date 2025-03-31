#include "scanline.h"

void
Seg::SetLRoom (std::shared_ptr<Room> room) {
    auto r = (room? room->Get() : nullptr);
    if (_lbRoom != r) {
        _lbRoom = r;
    }
}

void
Seg::SetRRoom (std::shared_ptr<Room> room) {
    auto r = (room? room->Get() : nullptr);
    if (_rtRoom != r) {
        _rtRoom = r;
    }
}

/*
 * RMGraph impl
 */
template <typename RoomT, typename SegT>
RMGraph<RoomT, SegT>::~RMGraph ()
{
    for (auto p : _segMap) {
        delete p->second; // seg
    }
}

template <typename RoomT, typename SegT>
std::shared_ptr<RoomT>
RMGraph<RoomT, SegT>::AllocRoom () noexcept
{
    auto room = std::make_shared<RoomT>(_nextRoomId);
#if DEBUG
    printf("== new room %lu allocated %p\n", _nextRoomId, room.get());
#endif
    _roomList.push_back(room);
    _nextRoomId += 1;
    return room;
}

template <typename RoomT, typename SegT>
std::shared_ptr<RoomT>
RMGraph<RoomT, SegT>::MergeRooms (std::shared_ptr<RoomT> lhs,
                                  std::shared_ptr<RoomT> rhs) noexcept
{
    assert(lhs && rhs);
    if (lhs == rhs) {
        return lhs;
    }

#if DEBUG
    printf("\tmerge Room-%lu to Room-%lu\n",
            rhs->GetId(), lhs->GetId());
#endif

    // merge room rhs to room lhs
    rhs->_parent = lhs; // TODO: delete this line?
    lhs->AddSegs(rhs->_segs);
    decltype(rhs->_segs){}.swap(rhs->_segs);

    return lhs;
}

template <typename Graph>
std::vector<Seg*>
RMGraphBuilder<Graph>::SplitToNonOverlapSegs (const std::vector<SegLoc>& iSegLocs)
{
    std::vector<Seg*> outSegs;
    
    // index -> segLoc
    std::vector<std::pair<size_t, SegLoc>> oSegLocs;
    gtl::intersect_segments(oSegLocs, iSegLocs.begin(), iSegLocs.end());
    
    // extract the common code of creating and setting segments
    auto createAndSetupSeg =
        [this, &outSegs, &iSegLocs](int idx, const SegLoc& segLoc)
        {
            bool isNewSeg;
            auto seg = this->_graph.GocSeg(segLoc, &isNewSeg);
            assert(seg);
            seg->SetTypeFrLoc(iSegLocs[idx].GetLBType(),
                              iSegLocs[idx].GetRTType());
            if (isNewSeg) {
                outSegs.push_back(seg);
            }
            return seg;
        };
            
    // Iterate through all segments after intersection processing
    for (auto iter = oSegLocs.begin(); iter != oSegLocs.end();)
    {
        const auto& [idx, segLoc] = *iter;
        // Check if the current segment has been modified during intersection
        if (segLoc != iSegLocs[idx])
        {
            // This segment has been split - create and set up the first part
            createAndSetupSeg(idx, segLoc);
            
            // Look ahead for additional segments with the same index
            auto idxIter = iter + 1;
            // Handle all segments that came from the same index
            for (; idxIter != oSegLocs.end();) {
                const auto& [i, nSegLoc] = *idxIter;
                if (i == idx) {
                    // Found another part of the same index
                    createAndSetupSeg(idx, nSegLoc);
                    idxIter++;
                } else {
                    // Found a segment from a different index
                    break;
                }
            }
            iter = idxIter;
        } else {
            // The segment was not split during intersection
            // (its geometry remains unchanged from the original)
            createAndSetupSeg(idx, segLoc);
            iter++;
        }
    }
    
    return outSegs;
}

template class RMGraphBuilder<RMGraph<Room, Seg>>;