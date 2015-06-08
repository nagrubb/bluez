extern "C" {
  #include "mainloop.h"
}

#include "MainLoop.h"
#include <iostream>

using namespace bluez::native;
using namespace std;

MainLoop* MainLoop::s_instance = NULL;
uint8_t MainLoop::s_refCount = 0;

MainLoop::MainLoop() {
  cout << "mainloop_init() started" << endl;
  mainloop_init();
  cout << "mainloop_init() ended" << endl;
  m_thread = boost::thread(&MainLoop::runner, this);
}

MainLoop::~MainLoop() {
  cout << "mainloop_quit() started" << endl;
  mainloop_quit();
  cout << "mainloop_quit() ended" << endl;
  m_thread.join();
}

void MainLoop::ref() {
  __sync_fetch_and_add(&s_refCount, 1);
}

void MainLoop::unref() {
  if (__sync_sub_and_fetch(&s_refCount, 1))
    return;

  if (s_instance != NULL) {
    delete s_instance;
    s_instance = NULL;
  }
}

void MainLoop::runner() {
  cout << "mainloop_run() started" << endl;
  mainloop_run();
  cout << "mainloop_run() ended" << endl;
}
