#include "LITTLEFS_LIB.h"

/**
 * @brief Dosya sistemini başlatır.
 * 
 * Bu fonksiyon LittleFS dosya sistemini başlatmayı dener.
 * Başarılı olursa true döner, aksi takdirde false.
 * Genelde setup() içinde bir kere çağrılır.
 * 
 * @return true  - Başlatma başarılı
 * @return false - Başlatma başarısız
 */
bool Dosya::kur()
{
    if (!LittleFS.begin())
    {
        Serial.print(F("\nLittleFS baslatilamadi!\n"));
        return false;
    }
    Serial.print(F("\nlittleFs baslatildi\n"));
    return true;
}

/**
 * @brief Belirtilen dosyanın mevcut olup olmadığını kontrol eder.
 * 
 * @param isim Kontrol edilecek dosyanın yolu
 * @return true Dosya mevcutsa
 * @return false Dosya yoksa
 */
bool Dosya::var_mi(const char* isim) {
    return LittleFS.exists(isim);
}

/**
 * @brief Dosya sistemini kapatır.
 * 
 * LittleFS'i sonlandırır. Genellikle sistem kapatılmadan önce çağrılır.
 */
void Dosya::kapat() {
    LittleFS.end();
    Serial.println(F("LittleFS kapatildi"));
}

/**
 * @brief Dosya yoksa boş dosya oluşturur.
 * 
 * @param isim Dosya yolu
 * @return true Dosya başarıyla oluşturuldu veya zaten var
 * @return false Dosya oluşturulamadı
 */
bool Dosya::olustur(const char* isim) {
    if (LittleFS.exists(isim)) {
        return true; // Zaten var
    }
    File dosya = LittleFS.open(isim, "w");
    if (!dosya) return false;
    dosya.close();
    return true;
}


/**
 * @brief Belirtilen dosyanın boyutunu döndürür.
 * 
 * Dosya başarıyla açılırsa, bayt cinsinden boyutunu döndürür.
 * 
 * @param isim Dosyanın yolu
 * @return int32_t Dosya boyutu (bayt), dosya açılamazsa -1
 */
int32_t Dosya::boyut(const char* isim) {
    File dosya = LittleFS.open(isim, "r");
    if (!dosya) return -1;
    int32_t size = dosya.size();
    dosya.close();
    return size;
}

/**
 * @brief Kullanılan alan yüzdesini yazdırır.
 * 
 * Dosya sisteminde ne kadar yer kullanıldığını yüzde olarak yazdırır.
 * Ayrıntı istemeyen durumlarda kullanılır.
 */
void Dosya::boyut()
{
    FSInfo fs_info;
    LittleFS.info(fs_info);
    float usedPercentage = (float(fs_info.usedBytes) / float(fs_info.totalBytes)) * 100;
    Serial.print(F("Kullanılan depolama boyutu: %") + String(usedPercentage) + "\n");
}

/**
 * @brief Depolama alanı hakkında ayrıntılı bilgi verir.
 * 
 * Toplam kapasite, kullanılan alan, boş alan, blok boyutu, 
 * sayfa boyutu, açık dosya limiti gibi bilgileri detaylı olarak yazdırır.
 */
void Dosya::boyut_genel_ayrintili()
{
    FSInfo info;
    LittleFS.info(info);
    size_t bosAlan = info.totalBytes - info.usedBytes;
    float oran_toplam = 100.0;
    float oran_kullanilan = ((float)info.usedBytes / info.totalBytes) * 100.0;
    float oran_bos = ((float)bosAlan / info.totalBytes) * 100.0;
    Serial.print(F("===== LittleFS Dosya Sistemi Bilgisi =====\n"));
    Serial.print(F("Toplam kapasite (flash'ta ayrılan alan)    : "));
    Serial.print(info.totalBytes);
    Serial.print(F(" bayt (%"));
    Serial.print(oran_toplam);
    Serial.print(F(")\n"));
    Serial.print(F("Kullanılan alan                           : "));
    Serial.print(info.usedBytes);
    Serial.print(F(" bayt (%"));
    Serial.print(oran_kullanilan);
    Serial.print(F(")\n"));
    Serial.print(F("Kullanılabilir (boş) alan                 : "));
    Serial.print(bosAlan);
    Serial.print(F(" bayt (%"));
    Serial.print(oran_bos);
    Serial.print(F(")\n"));
    Serial.print(F("Flash blok (silme) boyutu                 : "));
    Serial.print(info.blockSize);
    Serial.print(F(" bayt\n"));
    Serial.print(F("Flash sayfa (Serial.printma) boyutu                : "));
    Serial.print(info.pageSize);
    Serial.print(F(" bayt\n"));
    Serial.print(F("Aynı anda açık dosya limiti               : "));
    Serial.print(info.maxOpenFiles);
    Serial.print(F(" adet\n"));
    Serial.print(F("Maksimum dosya yolu uzunluğu              : "));
    Serial.print(info.maxPathLength);
    Serial.print(F(" karakter\n"));
    Serial.print(F("===========================================\n"));
}

/**
 * @brief Belirli bir dizindeki dosya ve klasörleri listeler.
 * 
 * Bu fonksiyon belirtilen klasör içeriğini listeler. Eğer alt klasörler varsa
 * onlar da girintili şekilde yazdırılır. Genelde test veya hata ayıklama amaçlıdır.
 * 
 * @param dizinYolu Listelenecek klasörün yolu (örn: "/")
 * @param seviye Girinti seviyesi, genelde sıfırdan başlar
 */
void Dosya::listele(const char *dizinYolu, int seviye)
{
    File klasor = LittleFS.open(dizinYolu, "r");
    if (!klasor || !klasor.isDirectory())
    {
        Serial.print(F("HATA: Geçerli klasör açılamadı: "));
        Serial.print(dizinYolu);
        Serial.print(F("\n"));
        return;
    }
    File girdi = klasor.openNextFile();
    while (girdi)
    {
        for (int i = 0; i < seviye; i++)
            Serial.print(F("  "));

        if (girdi.isDirectory())
        {
            Serial.print(F("[Klasör] "));
            Serial.print(girdi.name());
            Serial.print(F("/\n"));
            listele(girdi.name(), seviye + 1);
        }
        else
        {
            Serial.print(F("[Dosya ] "));
            Serial.print(girdi.name());
            Serial.print(F(" - "));
            Serial.print(girdi.size());
            Serial.print(F(" bayt\n"));
        }
        girdi = klasor.openNextFile();
    }
}

/**
 * @brief Kök dizinden başlayarak tüm dosya ve klasörleri listeler.
 * 
 * Root ("/") klasöründen başlar, tüm içeriği yazdırır.
 * listele() fonksiyonunu "/" ile çağırır.
 */
void Dosya::listele_tumicerik()
{
    Serial.print(F("===== LittleFS Tum Dosyalar =====\n"));
    listele("/");
    Serial.print(F("===== LittleFS Tum Dosyalar =====\n"));
}

/**
 * @brief Belirtilen dosyadaki satır sayısını sayar.
 * 
 * Dosya açılır ve içindeki tüm karakterler taranır.
 * Her `\n` karakteri yeni bir satır olarak kabul edilir.
 * 
 * @param isim Satır sayısı hesaplanacak dosyanın yolu
 * @return int16_t - Satır sayısı (başarılıysa), -1 (dosya açılamazsa)
 */
int16_t Dosya::satir_hesap(const char *isim)
{
    File dosya = LittleFS.open(isim, "r");
    if (!dosya || dosya.isDirectory())
    {
        Serial.print(F("Dosya veya klasor acilamadi"));
        return -1; // hata
    }
    int16_t satirSayisi = 0;
    while (dosya.available())
    {
        char karakter = dosya.read();
        if (karakter == '\n')
        {
            satirSayisi++;
        }
    }
    dosya.close();
    return satirSayisi;
}
