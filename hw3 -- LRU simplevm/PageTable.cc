#include "./PageTable.h"
#include<iostream>

namespace simplevm {
  // TODO: implment PageTable member functions
  PageTable::PageTable(std::string swap_file_name, size_t page_capacity) {
    swap_file_.open(swap_file_name, std::fstream::in | std::fstream::out);
    capacity_ = page_capacity;
  }

  PageTable::~PageTable() {
    flush_all_pages();
    for (auto& itr : page_map_) {
      delete itr.second;
    }
    swap_file_.close();
  }

  Page& PageTable::get_page(uint32_t virtual_address) {
    pno_t page_number = (virtual_address & 0xFFFFF000) >> 12;
    // 1. The page is currently in the "loaded" and in the cache.
    //    In this case, a reference to the page is returned and
    //    and the page is marked as most recently used in the cache
    if (page_map_.find(page_number) != page_map_.end()) {
      // itr is a pointer pointing to the element
      auto itr = page_map_.find(page_number);
      Page* find_page = itr->second;
      // return a reference to the page
      Page& ref = *find_page;
      
      // mark the page as most recently used
      page_lst_.remove(*(find_page));
      page_lst_.push_back(*(find_page));

      return ref;
    } else {
      if (loaded_pages() == capacity()) {
        // 3. The page is not currently "loaded", and the PageTable is at page capacity:
        //    The least recently used page in the cache is evicted from the cache.
        evict_page();
      } 
      // the page is loaded from the swap file and added to the cache as the most recently used page.
      Page* new_page = new Page(swap_file_, page_number);
      page_map_.insert (std::pair<pno_t, Page*>(page_number, new_page));
      page_lst_.push_back(*(new_page));
      return *new_page;
    }
  }

  size_t PageTable::capacity() {
    return capacity_;
  }

  size_t PageTable::loaded_pages() {
    return page_map_.size();
  }

  bool PageTable::page_available(pno_t virtual_pno) {
    for (auto& itr : page_map_) {
      if (itr.first == virtual_pno) {
        return true;
      }
    }
    return false;
  }

  void PageTable::flush_all_pages() {
    for (auto& itr : page_map_) {
      flush_page(itr.first);
    }
  }

  void PageTable::flush_page(pno_t virtual_pno) {
    for (auto& itr : page_map_) {
      if (itr.first == virtual_pno) {
        (itr.second)->flush();
      }
    }
  }

  void PageTable::discard_page(pno_t virtual_pno) {
    for (auto& itr : page_map_) {
      pno_t page_num = itr.first;
      Page* curr = itr.second;
      if (page_num == virtual_pno) {
        flush_page(page_num);
        page_lst_.remove(*curr);
        page_map_.erase(virtual_pno);
        delete curr; 
        return;
      }
    }
  }

  void PageTable::evict_page() {
    // get the first element(which is the least recently used)
    if (loaded_pages() == 0) {
      return;
    }
    pno_t virtual_pno = page_lst_.front().pno();
    discard_page(virtual_pno);
  } 
}
