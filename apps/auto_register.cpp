/**
 *  Copyright 2026 Mike Reed
 */

#include "auto_register.h"

static std::vector<GRegistrant*> gList;

GRegistrant::GRegistrant() {
    gList.push_back(this);
    printf("reg++ %zu\n", gList.size());
}

GRegistrant::~GRegistrant() {
    auto iter = std::find(gList.begin(), gList.end(), this);
    assert(iter != gList.end());
    gList.erase(iter);

    printf("reg-- %zu\n", gList.size());
}

size_t GRegistrant::Count() { return gList.size(); }

GRegistrant* GRegistrant::Get(size_t index) {
    assert(index < gList.size());
    return gList[index];
}
