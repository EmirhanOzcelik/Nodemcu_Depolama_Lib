#pragma once
#include "Arduino.h"
#include "LittleFS.h"

class Dosya
{
public:
    bool kur();
    bool var_mi(const char *isim);
    void kapat();
    bool olustur(const char *isim);
    int32_t boyut(const char *isim);
    void boyut();
    void boyut_genel_ayrintili();
    void listele(const char *dizinYolu, int seviye = 0);
    void listele_tumicerik();
    int16_t satir_hesap(const char *isim);
    String satir_oku(const char *isim, uint16_t satirNo);
    String oku(const char *isim);
    String oku(const char *isim, uint16_t ilkSatir, uint16_t sonSatir = 0xFFFF);
    bool satirOku(File &dosya, String &satir);
    bool yedekle(const char *dosyaAdi);
    bool geri_yukle(const char *dosyaAdi);
    int16_t ara(const char *isim, const String &aranan);
    bool satir_degistir(const char *isim, uint16_t satirNo, const String &yeniIcerik);
    bool satir_ekle(const char *isim, uint16_t konum, const String &veri);
    bool yaz(const char *isim, const String &veri);
    bool yaz(const char *isim, int veri);
    bool yaz(const char *isim, float veri);
    void degistir(const char *isim, const String &yeniIcerik);
    bool yeniden_adlandir(const char *eskiIsim, const char *yeniIsim);
    bool kopyala(const char *kaynak, const char *hedef);
    bool dizin_olustur(const char *yol);
    bool bos_mu(const char *yol);
    void degistir(const char *isim, const char *yeniIcerik);
    void degistir(const char *isim, int yeniIcerik);
    void degistir(const char *isim, float yeniIcerik);
    void ekle(const char *isim, const String &yeniIcerik);
    void ekle(const char *isim, const char *yeniIcerik);
    void ekle(const char *isim, int yeniIcerik);
    void ekle(const char *isim, float yeniIcerik);
    void sil_full_dizin(const char *dizinYolu);
    void sil(const char *dosyaAdi);
    void sil(const char *dosyaAdi, uint16_t ilkSatir, uint16_t sonSatir = 0xFFFF);
    bool satir_sil(const char *isim, uint16_t satirNo);
    void temizle(const char *isim);
    void test();
};