//
// Created by Andrzej Borucki on 2022-08-31
//

#ifndef CACHE_SLOTMAP_HPP
#define CACHE_SLOTMAP_HPP

#include <vector>
#include <iostream>
#include "murmur.h"
#include "SlotBits.hpp"

template<typename slot_t, typename K, typename V>
class  SlotMapT {
    struct Slot {
        K key;
        V value;
        slot_t next;
    };

    slot_t capacity;
    Slot *slots;
    SlotBits<slot_t> *bits;
    SlotBits<slot_t> *ebits;

    slot_t stat_slotlen(slot_t nSlot) {
        if (bits->isSlotFree(nSlot)) return 0;
        slot_t slotlen = 0;
        while (nSlot) {
            Slot &slot = slots[nSlot];
            slotlen++;
            nSlot = slot.next;
        }
        return slotlen;
    }

    slot_t findNFrom(const K key, slot_t nSlot) {
        while (nSlot) {
            Slot &slot = slots[nSlot];
            if (slot.key==key) return nSlot;
            nSlot = slot.next;
        }
        return 0;
    }

    Slot* findFromSlot(slot_t nFoundSlot) {
        if (!nFoundSlot)
            return nullptr;
        Slot &slot = slots[nFoundSlot];
        return &slot;
    }

    Slot* findFrom(const K key, slot_t nSlot){
        return findFromSlot(findNFrom(key, nSlot));
    }

    slot_t findN(const K key) {
        return findNFrom(key, startSlot(key));
    }

public:
    explicit SlotMapT(slot_t capacity): capacity(capacity) {
        bits = new SlotBits<slot_t>(capacity);
        ebits = new SlotBits<slot_t>(capacity);
        slots = new Slot[capacity + 1];
    }

    ~SlotMapT() {
        delete []slots;
        delete ebits;
        delete bits;
    }

    slot_t size() {
        return ebits->availCount-bits->availCount;
    }

    slot_t startSlot(const K key) {
        auto hash = murmur3_32(&key,sizeof(key));
        return (slot_t)(hash % capacity) + 1;
    }

    Slot* find(const K key) {
        slot_t slot = findNFrom(key, startSlot(key));
        if (bits->isSlotFree(slot)) return nullptr;
        if (ebits->isSlotOccupied(slot)) return nullptr;
        return findFromSlot(slot);
    }

    void erase(const K key) {
        auto nSlot = findN(key);
        if (nSlot)
            ebits->setAsErased(nSlot);
    }

    bool put(K key, V value) {
        auto hash = murmur3_32(&key,sizeof(key));
        slot_t nSlot = (slot_t)(hash % capacity) + 1; //0 is reserved as nullptr
        slot_t nSlot2 = 0;
        if (bits->isSlotOccupied(nSlot)) {
            Slot *slot = findFrom(key, nSlot);
            if (slot) { //replace value
                slot->value = value;
                return true;
            }
            nSlot2 = bits->findNextSlot(nSlot);
            if (nSlot2<0)
                return false;
            slots[nSlot2] = slots[nSlot];
            bits->setAsOccupied(nSlot2);
        } else
            bits->setAsOccupied(nSlot);
        Slot &slot = slots[nSlot];
        slot.key = key;
        slot.value = value;
        slot.next = nSlot2;
        return true;
    }

    void printStat() {
        slot_t maxlen = 0;
        for (slot_t i=0; i < capacity; i++)
            maxlen = std::max(maxlen, stat_slotlen(i));
        std::vector<slot_t> hist(maxlen+1);
        for (slot_t i=0; i < capacity; i++)
            hist[stat_slotlen(i)]++;
        slot_t sum = 0, count = 0;
        for (slot_t i=0; i<hist.size(); i++) {
            printf("%d: %f\n", i, double(hist[i]) / capacity);
            if (i>0) {
                sum += i * hist[i];
                count+=hist[i];
            }
        }
        printf("average %f\n",double(sum)/count);
    }
};

template<typename K, typename V> using SlotMap = SlotMapT<uint32_t, K,V>;
template<typename K, typename V> using SlotMap16 = SlotMapT<uint16_t, K,V>;

#endif //CACHE_SLOTMAP_HPP