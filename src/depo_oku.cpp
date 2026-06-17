#include "LITTLEFS_LIB.h"


/**
 * @brief Dosyadan belirtilen satırı okur.
 * 
 * @param isim Dosya yolu
 * @param satirNo Okunacak satır numarası (0 tabanlı)
 * @return String Okunan satır içeriği, satır yoksa boş string döner
 */
String Dosya::satir_oku(const char* isim, uint16_t satirNo) {
    File dosya = LittleFS.open(isim, "r");
    if (!dosya) return String();
    
    uint16_t sayac = 0;
    String satir = "";
    while (dosya.available()) {
        char c = dosya.read();
        if (c == '\n') {
            if (sayac == satirNo) {
                dosya.close();
                return satir;
            }
            satir = "";
            sayac++;
        } else {
            if (sayac == satirNo) {
                satir += c;
            }
        }
    }
    // Dosyanın son satırı ise ve '\n' yoksa
    if (sayac == satirNo) {
        dosya.close();
        return satir;
    }
    dosya.close();
    return String();
}



/**
 * @brief Bir dosyanın tüm içeriğini okur.
 *
 * Belirtilen dosyayı açar ve içindeki tüm satırları okuyarak tek bir String'e ekler.
 * Dosya açılamazsa boş string döner.
 *
 * @param isim Okunacak dosyanın yolu ("/veri.txt" gibi)
 * @return String Dosyanın tüm içeriği (satırlar dahil)
 */
String Dosya::oku(const char *isim)
{
    String dosyaIcerigi = "";
    File dosya = LittleFS.open(isim, "r");

    if (!dosya || dosya.isDirectory())
    {
        Serial.print(F("Dosya açılamadı"));
        return "";
    }

    while (dosya.available())
    {
        String satir = dosya.readStringUntil('\n');
        dosyaIcerigi += satir + "\n";
    }

    dosya.close();
    return dosyaIcerigi;
}

/**
 * @brief Bir dosyadaki belirli satır aralığını okur.
 *
 * Örneğin 3. satırdan 6. satıra kadar olan içerik alınabilir.
 * Eğer `sonSatir` 0xFFFF ise sadece `ilkSatir` okunur.
 * Hatalı aralık verilirse boş string döner.
 *
 * @param isim Okunacak dosyanın yolu
 * @param ilkSatir Başlangıç satırı (0'dan başlar)
 * @param sonSatir Bitiş satırı (dahil)
 * @return String Belirtilen satırlar birleşik olarak döner
 */
String Dosya::oku(const char *isim, uint16_t ilkSatir, uint16_t sonSatir)
{
    if (sonSatir == 0xFFFF)
        sonSatir = ilkSatir;

    String dosyaIcerigi = "";
    int16_t toplamSatir = satir_hesap(isim);

    if (toplamSatir < 0 || ilkSatir >= toplamSatir)
    {
        Serial.print(F("Geçersiz satır aralığı"));
        return "";
    }

    if (sonSatir >= toplamSatir)
        sonSatir = toplamSatir - 1;

    File dosya = LittleFS.open(isim, "r");

    if (!dosya || dosya.isDirectory())
    {
        Serial.print(F("Dosya açılamadı"));
        return "";
    }

    uint16_t mevcutSatir = 0;
    while (dosya.available())
    {
        String satir = dosya.readStringUntil('\n');
        if (mevcutSatir >= ilkSatir && mevcutSatir <= sonSatir)
            dosyaIcerigi += satir + "\n";

        if (mevcutSatir++ > sonSatir)
            break;
    }

    dosya.close();
    return dosyaIcerigi;
}

/**
 * @brief Satır okuyucu yardımcı fonksiyon.
 *
 * Tek bir satırı okur ve içeriği parametreye yazar.
 *
 * @param dosya Açık dosya referansı
 * @param satir Okunan satır verisini içerir
 * @return true Satır varsa, false boşsa
 */
bool Dosya::satirOku(File &dosya, String &satir)
{
    satir = dosya.readStringUntil('\n');
    return (satir.length() > 0);
}

/**
 * @brief Dosyayı .bak uzantısıyla yedekler.
 *
 * Örneğin "/veri.txt" dosyası "/veri.txt.bak" olarak yedeklenir.
 *
 * @param dosyaAdi Yedeklenecek dosyanın yolu
 * @return true Yedekleme başarılıysa
 */
bool Dosya::yedekle(const char *dosyaAdi)
{
    String yedekAdi = String(dosyaAdi) + ".bak";
    if (!LittleFS.exists(dosyaAdi)) return false;
    if (LittleFS.exists(yedekAdi.c_str())) LittleFS.remove(yedekAdi.c_str());
    return kopyala(dosyaAdi, yedekAdi.c_str());
}

/**
 * @brief Yedeği asıl dosyayla değiştirir.
 *
 * .bak dosyasını asıl dosyayla değiştirerek geri yükleme işlemi yapar.
 *
 * @param dosyaAdi Asıl dosyanın yolu
 * @return true Geri yükleme başarılıysa
 */
bool Dosya::geri_yukle(const char *dosyaAdi)
{
    String yedekAdi = String(dosyaAdi) + ".bak";
    if (!LittleFS.exists(yedekAdi.c_str())) return false;
    if (LittleFS.exists(dosyaAdi)) LittleFS.remove(dosyaAdi);
    return kopyala(yedekAdi.c_str(), dosyaAdi);
}

/**
 * @brief Dosyada belirtilen satırı arar.
 *
 * Belirtilen dosya içinde “aranan” stringi arar.
 * Eşleşme bulunduğunda satır numarası döner.
 *
 * @param isim Dosya yolu
 * @param aranan Aranacak içerik
 * @return int16_t Bulunursa satır numarası, bulunamazsa -1
 */
int16_t Dosya::ara(const char *isim, const String &aranan)
{
    File dosya = LittleFS.open(isim, "r");
    if (!dosya || dosya.isDirectory())
    {
        Serial.print(F("Dosya açılamadı\n"));
        return -1;
    }

    uint16_t mevcutSatir = 0;
    while (dosya.available())
    {
        String satir = dosya.readStringUntil('\n');
        if (satir == aranan)
        {
            dosya.close();
            return mevcutSatir;
        }
        mevcutSatir++;
    }

    dosya.close();
    return -1;
}
