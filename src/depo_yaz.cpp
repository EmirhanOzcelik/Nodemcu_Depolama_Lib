#include "LITTLEFS_LIB.h"

/**
 * @brief Belirli bir satırı yeni içerikle değiştirir.
 *
 * Dosya geçici olarak başka bir dosyaya kopyalanır ve
 * sadece hedef satır değiştirilerek tekrar kaydedilir.
 *
 * @param isim Değiştirilecek dosyanın adı
 * @param satirNo Değiştirilecek satır numarası (0 tabanlı)
 * @param yeniIcerik Yeni içerik
 * @return true Başarıyla değiştirildiyse
 * @return false Hata oluştuysa
 */
bool Dosya::satir_degistir(const char *isim, uint16_t satirNo, const String &yeniIcerik)
{
    if (!LittleFS.exists(isim))
        return false;

    File eski = LittleFS.open(isim, "r");
    if (!eski || eski.isDirectory())
        return false;

    const char *geciciIsim = "/__gecici__.tmp";
    File yeni = LittleFS.open(geciciIsim, "w");
    if (!yeni)
    {
        eski.close();
        return false;
    }

    String satir;
    uint16_t sayac = 0;
    while (satirOku(eski, satir))
    {
        if (sayac == satirNo)
            yeni.println(yeniIcerik);
        else
            yeni.println(satir);
        sayac++;
    }

    eski.close();
    yeni.close();

    // Eski dosyayı sil, geçiciyi yeni adla yeniden adlandır
    LittleFS.remove(isim);
    LittleFS.rename(geciciIsim, isim);
    return true;
}


/**
 * @brief Belirli bir satıra satır ekler (var olanlar kaydırılır).
 * 
 * @param isim Dosya yolu
 * @param konum Eklenecek satır numarası (0 tabanlı)
 * @param veri Eklenecek satır içeriği
 * @return true Başarılıysa true, aksi halde false
 */
bool Dosya::satir_ekle(const char* isim, uint16_t konum, const String& veri) {
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
        if (sayac == konum) {
            yeniIcerik += veri + "\n";
        }
        yeniIcerik += satir + "\n";
        sayac++;
    }
    // Eğer konum son satırdan büyükse, sona ekle
    if (konum >= sayac) {
        yeniIcerik += veri + "\n";
    }
    dosya.close();

    return yaz(isim, yeniIcerik);
}





/**
 * @brief Dosyanın içeriğini tamamen yazar (varsa eski içerik silinir).
 * 
 * @param isim Dosya yolu
 * @param veri Yazılacak metin
 * @return true Başarılıysa true, aksi halde false
 */
bool Dosya::yaz(const char* isim, const String& veri) {
    File dosya = LittleFS.open(isim, "w");
    if (!dosya) return false;
    dosya.print(veri);
    dosya.close();
    return true;
}

/** int versiyonu */
bool Dosya::yaz(const char* isim, int veri) {
    return yaz(isim, String(veri));
}

/** float versiyonu */
bool Dosya::yaz(const char* isim, float veri) {
    return yaz(isim, String(veri));
}



/**
 * @brief Dosyanın içeriğini tamamen silip, yeni içerik ile değiştirir.
 *
 * Verilen dosya yazma modunda açılır, eski içerik silinir ve `yeniIcerik` yazılır.
 *
 * @param isim Değiştirilecek dosyanın adı (örnek: "/ayar.txt")
 * @param yeniIcerik Dosyaya yazılacak yeni içerik (String olarak)
 */
void Dosya::degistir(const char *isim, const String &yeniIcerik)
{
    File dosya = LittleFS.open(isim, "w"); // 'w' = write, eski içerik silinir
    if (!dosya)
    {
        Serial.println(F("Dosya acilamadi"));
        return;
    }

    dosya.print(yeniIcerik); // Yeni içerik dosyaya yazılır
    dosya.close();
    Serial.println(F("icerik basariyla değistirildi"));
}

// Dosyayı başka bir isimle yeniden adlandırır
bool Dosya::yeniden_adlandir(const char *eskiIsim, const char *yeniIsim)
{
    if (!LittleFS.exists(eskiIsim))
        return false;
    if (LittleFS.exists(yeniIsim))
        return false; // hedef dosya varsa işlem yapma
    return LittleFS.rename(eskiIsim, yeniIsim);
}

// Dosya kopyalama (basit, küçük dosyalar için)
bool Dosya::kopyala(const char *kaynak, const char *hedef)
{
    File src = LittleFS.open(kaynak, "r");
    if (!src)
        return false;
    File dst = LittleFS.open(hedef, "w");
    if (!dst)
    {
        src.close();
        return false;
    }
    while (src.available())
    {
        dst.write(src.read());
    }
    src.close();
    dst.close();
    return true;
}

// Dizin oluşturur
bool Dosya::dizin_olustur(const char *yol)
{
    return LittleFS.mkdir(yol);
}

// Dizin boş mu kontrol eder (dosya varsa false döner)
bool Dosya::bos_mu(const char *yol)
{
    File dir = LittleFS.open(yol, "r");
    if (!dir || !dir.isDirectory())
        return false;
    File f = dir.openNextFile();
    bool bos = (f == false); // hiç dosya yoksa true
    if (f)
        f.close();
    dir.close();
    return bos;
}

/**
 * @brief String yerine const char* içeriği yazar.
 *
 * Bu fonksiyon, `String` versiyona yönlendirir.
 *
 * @param isim Dosya adı
 * @param yeniIcerik Yazılacak C-string (const char*)
 */
void Dosya::degistir(const char *isim, const char *yeniIcerik)
{
    degistir(isim, String(yeniIcerik));
}

/**
 * @brief Sayı değeri (int) dosyaya yazar (önce String'e çevrilir).
 *
 * @param isim Dosya adı
 * @param yeniIcerik Yazılacak tam sayı (int)
 */
void Dosya::degistir(const char *isim, int yeniIcerik)
{
    degistir(isim, String(yeniIcerik));
}

/**
 * @brief Ondalıklı sayı (float) dosyaya yazar (String'e çevrilerek).
 *
 * @param isim Dosya adı
 * @param yeniIcerik Yazılacak ondalıklı sayı (float)
 */
void Dosya::degistir(const char *isim, float yeniIcerik)
{
    degistir(isim, String(yeniIcerik));
}

/**
 * @brief Verilen içeriği dosyanın sonuna ekler.
 *
 * Dosya `append` modunda açılır. Var olan içerik korunur, yeni satır eklenir.
 *
 * @param isim Dosya adı
 * @param yeniIcerik Eklenecek içerik (String)
 */
void Dosya::ekle(const char *isim, const String &yeniIcerik)
{
    File dosya = LittleFS.open(isim, "a"); // 'a' = append (ekleme)
    if (!dosya)
    {
        Serial.println(F("Dosya acilamadi"));
        return;
    }
    dosya.print(yeniIcerik); // Yeni satır olarak ekle
    dosya.close();
    Serial.println(F("icerik basariyla eklendi"));
}

/**
 * @brief Dosyaya const char* olarak içerik ekler (String'e çevrilir).
 *
 * @param isim Dosya adı
 * @param yeniIcerik Eklenecek metin (char*)
 */
void Dosya::ekle(const char *isim, const char *yeniIcerik)
{
    ekle(isim, String(yeniIcerik));
}

/**
 * @brief Tam sayı değeri dosyanın sonuna ekler.
 *
 * @param isim Dosya adı
 * @param yeniIcerik Eklenecek int değer
 */
void Dosya::ekle(const char *isim, int yeniIcerik)
{
    ekle(isim, String(yeniIcerik));
}

/**
 * @brief Ondalıklı sayı değeri dosyanın sonuna ekler.
 *
 * @param isim Dosya adı
 * @param yeniIcerik Eklenecek float değer
 */
void Dosya::ekle(const char *isim, float yeniIcerik)
{
    ekle(isim, String(yeniIcerik));
}
