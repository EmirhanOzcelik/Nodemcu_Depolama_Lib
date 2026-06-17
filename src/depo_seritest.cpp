#include "LITTLEFS_LIB.h"

#define __TEST 1

// ─── Sabitler ────────────────────────────────────────────────────────────────
static const uint8_t MAX_LISTE = 24; // Bir dizinde gösterilecek max öğe
static const uint8_t YOL_MAX = 128;  // Yol uzunluğu limiti
static const uint8_t GIRDI_MAX = 96; // Girdi tamponu limiti

// ─── Mod tanımları ───────────────────────────────────────────────────────────
enum Mod
{
    MOD_GEZGIN,
    MOD_DOSYA,
    MOD_GIRDI
};

enum GirdiSebep
{
    GIRDI_YOK,
    GIRDI_KOPYALA,
    GIRDI_ADLANDIR,
    GIRDI_EKLE,
    GIRDI_ARA,
    GIRDI_SATIR_OKU,
    GIRDI_SATIR_DEG_NO,
    GIRDI_SATIR_DEG_ICERIK,
    GIRDI_YENI_DIZIN
};

// ─── Durum değişkenleri ──────────────────────────────────────────────────────
static bool ilk_calistirildi = false;
static Mod mod = MOD_GEZGIN;
static GirdiSebep girdi_sebep = GIRDI_YOK;

// Gezgin
static char mevcut_dizin[YOL_MAX] = "/";
static char liste_adlar[MAX_LISTE][64];
static bool liste_dizin_mi[MAX_LISTE];
static uint8_t liste_adet = 0;
static int8_t imleç = 0;

// Seçili dosya
static char secili[YOL_MAX] = "";
static uint8_t satir_sayac = 0; // L komutu için

// Girdi tamponu
static char girdi[GIRDI_MAX] = "";
static uint8_t girdi_uzun = 0;
static uint8_t degistir_satir_no = 0; // M komutu için

// ─── Yardımcı: yol birleştir ─────────────────────────────────────────────────
static void yol_birlestir(const char *diz, const char *ad, char *sonuc, uint8_t maks)
{
    if (strcmp(diz, "/") == 0)
        snprintf(sonuc, maks, "/%s", ad);
    else
        snprintf(sonuc, maks, "%s/%s", diz, ad);
}

// ─── Yardımcı: üst dizin ────────────────────────────────────────────────────
static void ust_dizin(const char *yol, char *sonuc, uint8_t maks)
{
    strncpy(sonuc, yol, maks);
    sonuc[maks - 1] = '\0';
    char *son = strrchr(sonuc, '/');
    if (son == NULL || son == sonuc)
        strcpy(sonuc, "/");
    else
        *son = '\0';
}

// ─── Dizin listesini doldur (doğrudan LittleFS API) ─────────────────────────
static void listeyi_doldur()
{
    liste_adet = 0;
    imleç = 0;

    File klasor = LittleFS.open(mevcut_dizin, "r");
    if (!klasor || !klasor.isDirectory())
        return;

    File girdi_f = klasor.openNextFile();
    while (girdi_f && liste_adet < MAX_LISTE)
    {
        const char *tam = girdi_f.name(); // "/dizin/dosya.txt" formatı

        // Sadece dosya/dizin adını al (son '/' sonrası)
        const char *sadece_ad = strrchr(tam, '/');
        sadece_ad = (sadece_ad != NULL) ? sadece_ad + 1 : tam;

        strncpy(liste_adlar[liste_adet], sadece_ad, 63);
        liste_adlar[liste_adet][63] = '\0';
        liste_dizin_mi[liste_adet] = girdi_f.isDirectory();
        liste_adet++;

        girdi_f.close();
        girdi_f = klasor.openNextFile();
    }
    klasor.close();
}

// ─── Ekran çizimi: gezgin ────────────────────────────────────────────────────
static void gezgin_ciz()
{
    Serial.println(F("\n+--------------------------------------+"));
    Serial.print(F("|  "));
    Serial.print(mevcut_dizin);
    Serial.println(F("  "));
    Serial.println(F("+--------------------------------------+"));

    if (liste_adet == 0)
    {
        Serial.println(F("|  (boş dizin)                         |"));
    }
    else
    {
        for (uint8_t i = 0; i < liste_adet; i++)
        {
            if (i == (uint8_t)imleç)
                Serial.print(F("|  > "));
            else
                Serial.print(F("|    "));

            Serial.print(liste_dizin_mi[i] ? F("[D] ") : F("[F] "));
            Serial.println(liste_adlar[i]);
        }
    }

    Serial.println(F("+--------------------------------------+"));
    Serial.println(F("| W:yukari  S:asagi  E:sec      B:geri |"));
    Serial.println(F("| N:yenidizin  Z:dizinsil  U:disk  H:? |"));
    Serial.println(F("+--------------------------------------+"));
}

// ─── Ekran çizimi: dosya menüsü ─────────────────────────────────────────────
static void dosya_menu_ciz()
{
    Serial.println(F("\n+--------------------------------------+"));
    Serial.print(F("|  Secili: "));
    Serial.println(secili);
    Serial.println(F("+--------------------------------------+"));
    Serial.println(F("| O:oku tumu    L:satir satir oku      |"));
    Serial.println(F("| I:bilgi       F:ara                  |"));
    Serial.println(F("| C:satir oku   M:satir degistir       |"));
    Serial.println(F("| J:satir ekle  T:icerik temizle       |"));
    Serial.println(F("| Y:yedekle     G:geri yukle           |"));
    Serial.println(F("| K:kopyala     A:adlandir             |"));
    Serial.println(F("| D:sil         X/B:geri don           |"));
    Serial.println(F("+--------------------------------------+"));
}

// ─── Girdi isteme ────────────────────────────────────────────────────────────
static void girdi_iste(const char *mesaj)
{
    Serial.println();
    Serial.print(F(">> "));
    Serial.print(mesaj);
    Serial.print(F(": "));
    girdi_uzun = 0;
    memset(girdi, 0, sizeof(girdi));
}

// ─── Yardım ──────────────────────────────────────────────────────────────────
static void yardim_goster()
{
    Serial.println(F("\n+========= YARDIM =========+"));
    Serial.println(F("| GEZGIN:                  |"));
    Serial.println(F("|  W/S   = yukari/asagi    |"));
    Serial.println(F("|  E     = gir / sec       |"));
    Serial.println(F("|  B     = ust dizin       |"));
    Serial.println(F("|  R     = yenile          |"));
    Serial.println(F("|  N     = yeni dizin      |"));
    Serial.println(F("|  Z     = dizini sil      |"));
    Serial.println(F("|  U     = disk bilgisi    |"));
    Serial.println(F("| DOSYA:                   |"));
    Serial.println(F("|  O L I F C M J           |"));
    Serial.println(F("|  T Y G K A D X           |"));
    Serial.println(F("+--------------------------+"));
}

/// @brief while içinde çağırın tüm dosya sistemini komutlarla kontrol etmenizi sağlar
void Dosya::test()
{
#if __TEST == 1

    if (!ilk_calistirildi)
    {
        ilk_calistirildi = true;
        mod = MOD_GEZGIN;
        strcpy(mevcut_dizin, "/");
        listeyi_doldur();
        Serial.println(F("\n==== LittleFS Dosya Gezgini ===="));
        Serial.println(F("Seri Monitor: 'No line ending' olmali!"));
        yardim_goster();
        gezgin_ciz();
        return;
    }

    if (!Serial.available())
        return;

    // =========================================================
    // MOD: GIRDI — kullanıcıdan metin bekleniyor
    // =========================================================
    if (mod == MOD_GIRDI)
    {
        char c = Serial.read();

        if (c == 'e' || c == 'E')
        {
            // E → girdiyi işle (onay tuşu)
            girdi[girdi_uzun] = '\0';
            Serial.println(girdi);

            char tam_yol[YOL_MAX];
            bool dosya_moduna_don = true;

            switch (girdi_sebep)
            {
            case GIRDI_KOPYALA:
                Serial.println(kopyala(secili, girdi) ? F("Kopyalandi ✓") : F("Hata ✗"));
                break;

            case GIRDI_ADLANDIR:
                yol_birlestir(mevcut_dizin, girdi, tam_yol, YOL_MAX);
                if (yeniden_adlandir(secili, tam_yol))
                {
                    strncpy(secili, tam_yol, YOL_MAX);
                    Serial.println(F("Adlandirildi ✓"));
                }
                else
                {
                    Serial.println(F("Hata ✗"));
                }
                break;

            case GIRDI_EKLE:
                ekle(secili, girdi);
                ekle(secili, "\n");
                Serial.println(); // ekle() zaten "icerik eklendi" yazıyor
                break;

            case GIRDI_ARA:
            {
                int16_t satir_no = ara(secili, String(girdi));
                if (satir_no >= 0)
                {
                    Serial.print(F("Bulundu → Satir: "));
                    Serial.println(satir_no);
                }
                else
                {
                    Serial.println(F("Bulunamadi ✗"));
                }
                break;
            }

            case GIRDI_SATIR_OKU:
                Serial.print(F("Satir "));
                Serial.print(atoi(girdi));
                Serial.print(F(": "));
                Serial.println(satir_oku(secili, (uint16_t)atoi(girdi)));
                break;

            case GIRDI_SATIR_DEG_NO:
                // İlk aşama: satır numarasını al, şimdi içeriği sor
                degistir_satir_no = (uint8_t)atoi(girdi);
                girdi_sebep = GIRDI_SATIR_DEG_ICERIK;
                girdi_iste("Yeni icerik");
                return; // henüz dosya moduna dönme

            case GIRDI_SATIR_DEG_ICERIK:
                Serial.println(
                    satir_degistir(secili, degistir_satir_no, String(girdi))
                        ? F("Degistirildi ✓")
                        : F("Hata ✗"));
                break;

            case GIRDI_YENI_DIZIN:
                yol_birlestir(mevcut_dizin, girdi, tam_yol, YOL_MAX);
                if (dizin_olustur(tam_yol))
                {
                    Serial.println(F("Dizin olusturuldu ✓"));
                }
                else
                {
                    Serial.println(F("Hata ✗"));
                }
                listeyi_doldur();
                mod = MOD_GEZGIN;
                girdi_sebep = GIRDI_YOK;
                gezgin_ciz();
                return;

            default:
                break;
            }

            girdi_sebep = GIRDI_YOK;
            mod = MOD_DOSYA;
            if (dosya_moduna_don)
                dosya_menu_ciz();
            return;
        }
        else if (c == 8 || c == 127) // Backspace
        {
            if (girdi_uzun > 0)
            {
                girdi_uzun--;
                Serial.print(F("\b \b"));
            }
        }
        else if (girdi_uzun < GIRDI_MAX - 1)
        {
            girdi[girdi_uzun++] = c;
            Serial.print(c); // echo
        }
        return;
    }

    char komut = Serial.read();
    if (komut == '\r' || komut == '\n')
        return; // gereksiz satır sonu karakterlerini yoksay

    // =========================================================
    // MOD: DOSYA — seçili dosya üzerinde işlem
    // =========================================================
    if (mod == MOD_DOSYA)
    {
        switch (komut)
        {
        // ── Tümünü oku ──
        case 'O':
        case 'o':
            Serial.println(F("\n-- Dosya Icerigi --"));
            Serial.println(oku(secili));
            Serial.println(F("-------------------"));
            break;

        // ── Satır satır oku ──
        case 'L':
        case 'l':
        {
            String satir = satir_oku(secili, satir_sayac);
            if (satir.length() == 0 && satir_sayac > 0)
            {
                Serial.println(F("(Dosya sonu — L ile basa sar)"));
                satir_sayac = 0;
            }
            else
            {
                Serial.print(satir_sayac);
                Serial.print(F(": "));
                Serial.println(satir);
                satir_sayac++;
            }
            return; // menüyü tekrar çizme
        }

        // ── Bilgi ──
        case 'I':
        case 'i':
            Serial.print(F("Boyut  : "));
            Serial.print(boyut(secili));
            Serial.println(F(" byte"));
            Serial.print(F("Satirlar: "));
            Serial.println(satir_hesap(secili));
            break;

        // ── Ara ──
        case 'F':
        case 'f':
            mod = MOD_GIRDI;
            girdi_sebep = GIRDI_ARA;
            girdi_iste("Aranacak kelime");
            return;

        // ── Belirli satır oku ──
        case 'C':
        case 'c':
            mod = MOD_GIRDI;
            girdi_sebep = GIRDI_SATIR_OKU;
            girdi_iste("Satir no (0'dan baslar)");
            return;

        // ── Satır değiştir ──
        case 'M':
        case 'm':
            mod = MOD_GIRDI;
            girdi_sebep = GIRDI_SATIR_DEG_NO;
            girdi_iste("Degistirilecek satir no");
            return;

        // ── Sona satır ekle ──
        case 'J':
        case 'j':
            mod = MOD_GIRDI;
            girdi_sebep = GIRDI_EKLE;
            girdi_iste("Eklenecek metin");
            return;

        // ── İçeriği temizle ──
        case 'T':
        case 't':
            temizle(secili);
            satir_sayac = 0;
            Serial.println(F("Icerik temizlendi ✓"));
            break;

        // ── Yedekle ──
        case 'Y':
        case 'y':
            Serial.println(yedekle(secili) ? F("Yedeklendi ✓") : F("Hata ✗"));
            break;

        // ── Geri yükle ──
        case 'G':
        case 'g':
            Serial.println(geri_yukle(secili) ? F("Geri yuklendi ✓") : F("Hata ✗"));
            break;

        // ── Kopyala ──
        case 'K':
        case 'k':
            mod = MOD_GIRDI;
            girdi_sebep = GIRDI_KOPYALA;
            girdi_iste("Hedef yol (orn: /kopya.txt)");
            return;

        // ── Yeniden adlandır ──
        case 'A':
        case 'a':
            mod = MOD_GIRDI;
            girdi_sebep = GIRDI_ADLANDIR;
            girdi_iste("Yeni ad");
            return;

        // ── Sil ──
        case 'D':
        case 'd':
            sil(secili);
            secili[0] = '\0';
            satir_sayac = 0;
            mod = MOD_GEZGIN;
            listeyi_doldur();
            gezgin_ciz();
            return;

        // ── Geri ──
        case 'X':
        case 'x':
        case 'B':
        case 'b':
            secili[0] = '\0';
            satir_sayac = 0;
            mod = MOD_GEZGIN;
            gezgin_ciz();
            return;

        default:
            return;
        }

        dosya_menu_ciz();
        return;
    }

    // =========================================================
    // MOD: GEZGIN — dizin listesinde gezinme
    // =========================================================
    bool yenile = false;

    switch (komut)
    {
    // ── Yukarı ──
    case 'W':
    case 'w':
        if (liste_adet > 0 && imleç > 0)
        {
            imleç--;
            yenile = true;
        }
        break;

    // ── Aşağı ──
    case 'S':
    case 's':
        if (liste_adet > 0 && imleç < (int8_t)(liste_adet - 1))
        {
            imleç++;
            yenile = true;
        }
        break;

    // ── Seç / Gir (E tuşu) ──
    case 'E':
    case 'e':
        if (liste_adet > 0)
        {
            if (liste_dizin_mi[imleç])
            {
                char yeni_yol[YOL_MAX];
                yol_birlestir(mevcut_dizin, liste_adlar[imleç], yeni_yol, YOL_MAX);
                strncpy(mevcut_dizin, yeni_yol, YOL_MAX);
                listeyi_doldur();
                yenile = true;
            }
            else
            {
                yol_birlestir(mevcut_dizin, liste_adlar[imleç], secili, YOL_MAX);
                satir_sayac = 0;
                mod = MOD_DOSYA;
                dosya_menu_ciz();
                return;
            }
        }
        break;

    // ── Geri ──
    case 'B':
    case 'b':
        if (strcmp(mevcut_dizin, "/") != 0)
        {
            char ust[YOL_MAX];
            ust_dizin(mevcut_dizin, ust, YOL_MAX);
            strncpy(mevcut_dizin, ust, YOL_MAX);
            listeyi_doldur();
            yenile = true;
        }
        else
        {
            Serial.println(F("Zaten kok dizindesin."));
        }
        break;

    // ── Yenile ──
    case 'R':
    case 'r':
        listeyi_doldur();
        yenile = true;
        break;

    // ── Yeni dizin ──
    case 'N':
    case 'n':
        mod = MOD_GIRDI;
        girdi_sebep = GIRDI_YENI_DIZIN;
        girdi_iste("Yeni dizin adi");
        return;

    // ── Mevcut dizini sil ──
    case 'Z':
    case 'z':
        if (strcmp(mevcut_dizin, "/") == 0)
        {
            Serial.println(F("Kok dizin silinemez!"));
        }
        else
        {
            sil_full_dizin(mevcut_dizin);
            ust_dizin(mevcut_dizin, mevcut_dizin, YOL_MAX);
            listeyi_doldur();
            yenile = true;
        }
        break;

    // ── Disk bilgisi ──
    case 'U':
    case 'u':
        Serial.println(F("\n-- Disk Kullanimi --"));
        boyut_genel_ayrintili();
        break;

    // ── Yardım ──
    case 'H':
    case 'h':
        yardim_goster();
        yenile = true;
        break;

    default:
        break;
    }

    if (yenile)
        gezgin_ciz();

#else
    Serial.println(F("TEST modu devre disi."));
#endif
}