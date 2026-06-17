#include "LITTLEFS_LIB.h"

/**
 * @brief Bir dizindeki tüm dosya ve klasörleri (alt klasörler dahil) siler.
 * 
 * Verilen dizin içerisindeki tüm içerikler sırayla silinir.
 * Alt dizinler varsa, onlar da iç içe olacak şekilde temizlenir.
 * Kök dizin ("/") silinemez ama içindekiler silinir.
 * 
 * @param dizinYolu Silinecek dizin yolu (örnek: "/veriler")
 */
void Dosya::sil_full_dizin(const char *dizinYolu)
{
    File root = LittleFS.open(dizinYolu, "r");
    if (!root || !root.isDirectory())
    {
        Serial.print(String(F("Gecersiz dizin: ")) + dizinYolu);
        return;
    }

    File dosya = root.openNextFile();
    while (dosya)
    {
        String yol = String(dosya.name());
        if (dosya.isDirectory())
        {
            Serial.print(String(F("Alt klasor bulundu: ")) + yol);
            sil_full_dizin(yol.c_str());
            if (LittleFS.rmdir(yol))
                Serial.print(String(F("Klasor silindi: ")) + yol);
            else
                Serial.print(String(F("Klasor silinemedi: ")) + yol);
        }
        else
        {
            Serial.print(String(F("Siliniyor: ")) + yol);
            if (LittleFS.remove(yol))
                Serial.print(String(F("Dosya silindi: ")) + yol);
            else
                Serial.print(String(F("Dosya silinemedi: ")) + yol);
        }
        dosya.close();
        dosya = root.openNextFile();
    }

    root.close();

    if (String(dizinYolu) != "/")
    {
        if (LittleFS.rmdir(dizinYolu))
            Serial.print(String(F("Dizin silindi: ")) + dizinYolu);
        else
            Serial.print(String(F("Dizin silinemedi: ")) + dizinYolu);
    }
    else
        Serial.print(String(F("Kok dizin silinemedi: ")) + dizinYolu);
}

/**
 * @brief Verilen dosyayı siler.
 * 
 * Eğer dosya mevcutsa silinir. Dosya yoksa veya silinemiyorsa hata mesajı yazdırılır.
 * 
 * @param dosyaAdi Silinecek dosyanın yolu (örnek: "/log.txt")
 */
void Dosya::sil(const char *dosyaAdi)
{
    if (LittleFS.exists(dosyaAdi))
    {
        if (LittleFS.remove(dosyaAdi))
            Serial.print(String(F("Silindi: ")) + dosyaAdi);
        else
            Serial.print(String(F("Dosya silinemedi: ")) + dosyaAdi);
    }
    else
        Serial.print(String(F("Dosya yok: ")) + dosyaAdi);
}

/**
 * @brief Bir dosyadaki belirli satır aralığını siler.
 * 
 * Dosya okunur, verilen satır aralığı (örnek: 2–4. satırlar) hariç kalan satırlar
 * geçici olarak saklanır. Daha sonra dosya tamamen silinir ve kalan satırlar tekrar yazılır.
 * 
 * @param dosyaAdi Düzenlenecek dosyanın yolu
 * @param ilkSatir Silinmeye başlanacak satır (0'dan başlar)
 * @param sonSatir Silinecek son satır (dahil). 0xFFFF verilirse yalnızca `ilkSatir` silinir.
 */
void Dosya::sil(const char *dosyaAdi, uint16_t ilkSatir, uint16_t sonSatir)
{
    int16_t toplamSatir = satir_hesap(dosyaAdi);
    if (toplamSatir <= 0 || ilkSatir >= toplamSatir)
    {
        Serial.print(F("Silme islemi gecersiz."));
        return;
    }

    if (sonSatir == 0xFFFF)
        sonSatir = ilkSatir;

    if (sonSatir >= toplamSatir)
        sonSatir = toplamSatir - 1;

    File orijinal = LittleFS.open(dosyaAdi, "r");
    if (!orijinal)
    {
        Serial.print(F("Dosya acilamadi"));
        return;
    }

    String yeniIcerik = "";
    uint16_t mevcutSatir = 0;

    while (orijinal.available())
    {
        String satir = orijinal.readStringUntil('\n');
        if (mevcutSatir < ilkSatir || mevcutSatir > sonSatir)
            yeniIcerik += satir + "\n";
        mevcutSatir++;
    }
    orijinal.close();

    LittleFS.remove(dosyaAdi);
    File yeniDosya = LittleFS.open(dosyaAdi, "w");
    if (yeniDosya)
    {
        yeniDosya.print(yeniIcerik);
        yeniDosya.close();
        Serial.print(F("Satirlar silindi."));
    }
    else
        Serial.print(F("Yeni dosya olusturulamadi."));
}


/**
 * @brief Belirli bir satırı siler.
 * 
 * @param isim Dosya yolu
 * @param satirNo Silinecek satır numarası (0 tabanlı)
 * @return true Başarılıysa true, aksi halde false
 */
bool Dosya::satir_sil(const char* isim, uint16_t satirNo) {
    File dosya = LittleFS.open(isim, "r");
    if (!dosya) return false;

    String yeniIcerik = "";
    uint16_t sayac = 0;
    while (dosya.available()) {
        String satir = "";
        while (dosya.available()) {
            char c = dosya.read();
            if (c == '\n') break;
            satir += c;
        }
        if (sayac != satirNo) {
            yeniIcerik += satir + "\n";
        }
        sayac++;
    }
    dosya.close();

    return yaz(isim, yeniIcerik);
}


/**
 * @brief Dosyanın içeriğini temizler (dosya silinmez).
 * 
 * @param isim Dosya yolu
 */
void Dosya::temizle(const char* isim) {
    File dosya = LittleFS.open(isim, "w");
    if (dosya) dosya.close();  // İçeriği siler (boş yazar)
}
