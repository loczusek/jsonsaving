#include <QtCore>
#include <QtGui>
#include <QObject>

using namespace std;

// Klasa reprezentuje pojedynczą myśl. Każda myśl ma tekstową treść i listę połączeń z innymi myślami.
class Mysl {
public:
    QString wpisz;
    QList<Mysl*> polaczmysli;

//Metoda zwraca obiekt JSON reprezentujący daną myśl. Jest to wykorzystywane podczas zapisywania myśli do pliku.
    QJsonObject toJSON() const {
        QJsonObject json;
        json["wpisz"] = wpisz;
        json["polaczmysli"] = QJsonArray::fromVariantList(qListToVariantList(polaczmysli));
        return json;
    }
//Metoda fromJSON() to metoda statyczna, która przyjmuje obiekt JSON i listę wszystkich myśli, a następnie tworzy nowy obiekt Thought zgodnie z danymi z JSON i dodaje go do listy myśli.
    static Mysl* fromJSON(const QJsonObject& json, QList<Mysl*>& wszystkiemysli) {
        Mysl* Mysl = new class Mysl;
        Mysl->wpisz = json["wpisz"].toString();
        wszystkiemysli << Mysl;
        for (const auto& connection : json["polaczmysli"].toArray()) {
            Mysl->polaczmysli << wszystkiemysli[connection.toInt()];
        }
        return Mysl;
    }

private:
    static QVariantList qListToVariantList(const QList<QObject*> &list) {
        QVariantList listawariantow;
        for (auto item : list) {
            listawariantow.append(QVariant::fromValue(item));
        }
        return listawariantow;
    }
};
//Klasa zarządza wszystkimi myślami. Zawiera listę wskaźników na wszystkie myśli oraz metody do dodawania, łączenia, zapisywania i odczytywania myśli z pliku.
class ManagerMysli {
public:
    ManagerMysli() {}
    ~ManagerMysli() {
        for (auto Mysl : m_wszystkiemysli) {
            delete Mysl;
        }
    }

    void DodajMysl(const QString& wpisz) {
        auto Mysl = new class Mysl;
        Mysl->wpisz = wpisz;
        m_wszystkiemysli << Mysl;
    }

    void PolaczMysl(Mysl* a, Mysl* b) {
        a->polaczmysli << b;
        b->polaczmysli << a;
    }

    void ZapiszDoPliku(const QString& fileName) const {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            QJsonArray thoughtsArray;
            for (auto Mysl : m_wszystkiemysli) {
                thoughtsArray.append(Mysl->toJSON());
            }
            QJsonObject json;
            json["Mysli"] = thoughtsArray;
            QJsonDocument doc(json);
            file.write(doc.toJson());
        }
    }

    void WczytajZPliku(const QString& fileName) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QJsonObject json = doc.object();
            auto thoughtsArray = json["Mysli"].toArray();
            for (const auto& thoughtJson : thoughtsArray) {
                auto Mysl = Mysl::fromJSON(thoughtJson.toObject(), m_wszystkiemysli);
                m_wszystkiemysli << Mysl;
            }
            for (auto thought : m_wszystkiemysli) {
                for (auto connection : thought->polaczmysli) {
                    if (!m_wszystkiemysli.contains(connection)) {
                        qWarning() << "Blad Podczas Wczytywania Pliku!";
                    }
                }
            }
        }
    }

    void printMysli() const {
        for (auto Mysl : m_wszystkiemysli) {
            qDebug() << "Mysl:" << Mysl->wpisz;
            qDebug() << "  Polaczone Mysli:";
            for (auto connection : Mysl->polaczmysli) {
                qDebug() << "    " << connection->wpisz;
            }
        }
    }

public:
    QList<Mysl*> m_wszystkiemysli;
};

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    ManagerMysli manager;

    // Dodaje kilka myśli
    manager.DodajMysl("Myśl 1");
    manager.DodajMysl("Myśl 2");
    manager.DodajMysl("Myśl 3");
    manager.DodajMysl("Myśl 4");

   // Połączam kilka myśli
   manager.PolaczMysl(manager.m_wszystkiemysli[0], manager.m_wszystkiemysli[1]);
   manager.PolaczMysl(manager.m_wszystkiemysli[1], manager.m_wszystkiemysli[2]);
   manager.PolaczMysl(manager.m_wszystkiemysli[2], manager.m_wszystkiemysli[3]);

   // Wyświetlam stan myśli
   qDebug() << "Stan przed zapisem:";
   manager.printMysli();

   // Zapisuje stan do pliku
   manager.ZapiszDoPliku("myśli.json");

   // Usuwam wszystkie myśli
   manager.~ManagerMysli();

   // Odtwarzam stan z pliku
   ManagerMysli loadedManager;
   loadedManager.WczytajZPliku("myśli.json");

   // Wyświetlam odtworzony stan myśli
   qDebug() << "Stan po odtworzeniu:";
   loadedManager.printMysli();

   return app.exec();}



//Zapisać do pliku wszystkie dane wymagane do odtworzenia stanu programu.
//Qt posiada przyjemne klasy do tworzenia i czytania plików JSON.
//mapa posiada napis, położenie, rozmiar oraz odnośniki do innych myśli. Przeiterować po wszystkich myślach, stworzyć obiekty QJsonObject zawierające opis myśli i wrzucić to do pliku.
//Podczas odczytu trzeba najpierw wczytać wszystkie myśli, umieścić je w odpowiednich miejscach i dopiero na końcu odtworzyć połączenia na podstawie wczytanych danych.
