#pragma once

#include "flow_Analyst.h"
#include <msclr/marshal_cppstd.h>
#include <thread>



namespace ARMV {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)// Конструктор, в котором инициализируем форму
		{
			InitializeComponent();
			flow = new nflow_Analyst::flow_Analyst();// Создаём объект flow для управления торговыми потоками
			currentState = 0;
			CreateInitialControls();
			//
			// TODO: добавить код инициализации
			//
		}

	protected:

		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}
        // Элементы управления формы
	System::Windows::Forms::TextBox^ textBox1;
        System::Windows::Forms::Button^ buttonNext;
        System::Windows::Forms::Button^ buttonNext2;
        System::Windows::Forms::Label^ label1;
        System::Windows::Forms::Label^ label2;

        System::Windows::Forms::CheckBox^ checkBox1;
        System::Windows::Forms::CheckBox^ checkBox2;
        System::Windows::Forms::CheckBox^ checkBox3;
        System::Windows::Forms::CheckBox^ checkBox4;
        System::Windows::Forms::Button^ buttonSubmit;

        int currentState;
        System::ComponentModel::Container^ components;

        void InitializeComponent(void)// Инициализация формы
        {
            this->SuspendLayout();
            //
            // MyForm
            //
            this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->BackColor = System::Drawing::Color::DimGray;
            this->ClientSize = System::Drawing::Size(800, 400);
            this->Margin = System::Windows::Forms::Padding(4);
            this->Name = L"MyForm";
            this->Text = L"Торговый бот";
            this->ResumeLayout(false);
            this->BackColor = System::Drawing::ColorTranslator::FromHtml("#282828");

        }

        void CreateInitialControls()// Создание элементов формы в зависимости от текущего состояния
        {
            // Очищаем форму
            this->Controls->Clear();
            int formWidth = this->ClientSize.Width / 2;
            if (currentState == 0)
            {

                // Создаём Label
                label1 = gcnew Label();
                label1->Text = L"ВВЕДИТЕ ТОКЕН";
                label1->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14, FontStyle::Regular); // Увеличенный размер шрифта
                label1->AutoSize = true;
                label1->ForeColor = System::Drawing::Color::White;
                // Автоматический размер под текст

                int labelWidth = label1->PreferredWidth/2;
                int xPos = formWidth - labelWidth;
                label1->Location = Point(xPos, 50);
                // Создаём TextBox
                textBox1 = gcnew TextBox();
                textBox1->Size = System::Drawing::Size(400, 150);
                int textBoxX = formWidth - (textBox1->Width) / 2; // Выравнивание по горизонтали
                textBox1->Location = Point(textBoxX, 140);

                // Настройка скрытия символов токена
                textBox1->PasswordChar = L'*'; // Символ-заглушка вместо вводимых символов
                textBox1->UseSystemPasswordChar = false; // Отключаем системный символ пароля


                // Создаём кнопку
                buttonNext = gcnew Button();
                buttonNext->Text = L"Далее";
                buttonNext->Size = System::Drawing::Size(200, 50);
                buttonNext->Location = Point(formWidth- buttonNext->Width/2, 220);
                buttonNext->Click += gcnew EventHandler(this, &MyForm::OnButtonNextClick);
                buttonNext->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14, FontStyle::Regular);
                buttonNext->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
                buttonNext->FlatAppearance->BorderSize = 0; // Убираем рамку
                buttonNext->BackColor = System::Drawing::ColorTranslator::FromHtml("#FF8243"); // Оранжевый цвет
                buttonNext->ForeColor = System::Drawing::Color::Black;
                // Создаём кнопку с закруглёнными углами
                System::Drawing::Drawing2D::GraphicsPath^ path = gcnew System::Drawing::Drawing2D::GraphicsPath();
                int cornerRadius = 15; // Радиус закругления
                ARMV::Rectangle rect = buttonNext->ClientRectangle;

                // Строим путь с закруглёнными углами
                path->AddArc(rect.X, rect.Y, cornerRadius, cornerRadius, 180, 90); // Левый верхний
                path->AddArc(rect.Right - cornerRadius, rect.Y, cornerRadius, cornerRadius, 270, 90); // Правый верхний
                path->AddArc(rect.Right - cornerRadius, rect.Bottom - cornerRadius, cornerRadius, cornerRadius, 0, 90); // Правый нижний
                path->AddArc(rect.X, rect.Bottom - cornerRadius, cornerRadius, cornerRadius, 90, 90); // Левый нижний
                path->CloseFigure();

                buttonNext->Region = gcnew System::Drawing::Region(path); // Применяем форму региона

                // Эффекты при взаимодействии
                buttonNext->FlatAppearance->MouseOverBackColor = System::Drawing::ColorTranslator::FromHtml("#FF9D65"); // Светлее при наведении
                buttonNext->FlatAppearance->MouseDownBackColor = System::Drawing::ColorTranslator::FromHtml("#E6743B"); // Темнее при нажатии
                buttonNext->Cursor = Cursors::Hand; // Курсор-рука при наведении
                // Добавляем элементы на форму
                this->Controls->Add(label1);
                this->Controls->Add(textBox1);
                this->Controls->Add(buttonNext);
            }
            else if (currentState == 1)
            {
                label2 = gcnew Label();
                label2->Text = L"Выберите акции:";
                label2->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14, FontStyle::Regular); // Увеличенный размер шрифта
                label2->AutoSize = true; // Автоматический размер под текст
                label2->ForeColor = System::Drawing::Color::White;
                int labelWidth = label2->PreferredWidth / 2;
                int xPos = formWidth - labelWidth;
                label2->Location = Point(xPos, 50);

                // Создаём чекбоксы для выбора акций

                checkBox1 = gcnew CheckBox();
                checkBox1->Text = L"Яндекс";
                checkBox1->Location = Point(210, 100);
                checkBox1->Size = System::Drawing::Size(200, 20);
                checkBox1->ForeColor = System::Drawing::Color::White;
                checkBox1->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, FontStyle::Regular);

                checkBox2 = gcnew CheckBox();
                checkBox2->Text = L"Сбербанк";
                checkBox2->Location = Point(210, 130);
                checkBox2->Size = System::Drawing::Size(200, 20);
                checkBox2->ForeColor = System::Drawing::Color::White;
                checkBox2->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, FontStyle::Regular);

                checkBox3 = gcnew CheckBox();
                checkBox3->Text = L"Whoosh";
                checkBox3->Location = Point(210, 160);
                checkBox3->Size = System::Drawing::Size(200, 20);
                checkBox3->ForeColor = System::Drawing::Color::White;
                checkBox3->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, FontStyle::Regular);

                checkBox4 = gcnew CheckBox();
                checkBox4->Text = L"МИХ";
                checkBox4->Location = Point(210, 190);
                checkBox4->Size = System::Drawing::Size(200, 20);
                checkBox4->ForeColor = System::Drawing::Color::White;
                checkBox4->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, FontStyle::Regular);


                buttonSubmit = gcnew Button();
                buttonSubmit->Text = L"Начать";
                buttonSubmit->Size = System::Drawing::Size(200, 50);
                buttonSubmit->Location = Point(formWidth - (buttonSubmit->Width) / 2, 250); // По центру
                buttonSubmit->Click += gcnew EventHandler(this, &MyForm::OnButtonSubmitClick);
                buttonSubmit->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14, FontStyle::Regular);

                buttonSubmit->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
                buttonSubmit->FlatAppearance->BorderSize = 0;

                buttonSubmit->BackColor = System::Drawing::Color::FromArgb(255, 0x82, 0x43);
                buttonSubmit->ForeColor = System::Drawing::Color::Black; // Чёрный текст

                System::Drawing::Drawing2D::GraphicsPath^ path = gcnew System::Drawing::Drawing2D::GraphicsPath();
                int cornerRadius = 15; // Радиус закругления

                path->AddArc(0, 0, cornerRadius, cornerRadius, 180, 90); // Левый верхний
                path->AddArc(buttonSubmit->Width - cornerRadius, 0, cornerRadius, cornerRadius, 270, 90); // Правый верхний
                path->AddArc(buttonSubmit->Width - cornerRadius, buttonSubmit->Height - cornerRadius, cornerRadius, cornerRadius, 0, 90); // Правый нижний
                path->AddArc(0, buttonSubmit->Height - cornerRadius, cornerRadius, cornerRadius, 90, 90); // Левый нижний
                path->CloseFigure();

                buttonSubmit->Region = gcnew System::Drawing::Region(path);

                buttonSubmit->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(255, 0x9D, 0x65); // Светлее при наведении
                buttonSubmit->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(255, 0x74, 0x3B); // Темнее при нажатии
                buttonSubmit->Cursor = Cursors::Hand;

                this->Controls->Add(label2);
                this->Controls->Add(checkBox1);
                this->Controls->Add(checkBox2);
                this->Controls->Add(checkBox3);
                this->Controls->Add(checkBox4);
                this->Controls->Add(buttonSubmit);
            }
            else if (currentState == 2)
            {
                buttonNext2 = gcnew Button();
                buttonNext2->Text = L"Завершить";
                buttonNext2->Size = System::Drawing::Size(200, 50);
                buttonNext2->Location = Point(formWidth - (buttonNext2->Width) / 2, 175); // По центру
                buttonNext2->Click += gcnew EventHandler(this, &MyForm::OnButtonNext2Click);
                buttonNext2->Font = gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14, FontStyle::Regular);

                // Настройка стиля кнопки
                buttonNext2->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
                buttonNext2->FlatAppearance->BorderSize = 0;

                // Оранжевый цвет кнопки (#FF8243)
                buttonNext2->BackColor = System::Drawing::Color::FromArgb(255, 0xFF, 0x82, 0x43);
                buttonNext2->ForeColor = System::Drawing::Color::White; // Белый текст

                // Закруглённые углы кнопки (15px радиус)
                System::Drawing::Drawing2D::GraphicsPath^ path = gcnew System::Drawing::Drawing2D::GraphicsPath();
                int cornerRadius = 15;

                // Добавляем закруглённые углы
                path->AddArc(0, 0, cornerRadius, cornerRadius, 180, 90); // Левый верх
                path->AddArc(buttonNext2->Width - cornerRadius, 0, cornerRadius, cornerRadius, 270, 90); // Правый верх
                path->AddArc(buttonNext2->Width - cornerRadius, buttonNext2->Height - cornerRadius, cornerRadius, cornerRadius, 0, 90); // Правый низ
                path->AddArc(0, buttonNext2->Height - cornerRadius, cornerRadius, cornerRadius, 90, 90); // Левый низ
                path->CloseFigure();

                // Применяем форму региона
                buttonNext2->Region = gcnew System::Drawing::Region(path);

                // Эффекты взаимодействия
                buttonNext2->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(255, 0xFF, 0x9D, 0x65); // Светлее при наведении
                buttonNext2->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(255, 0xE6, 0x74, 0x3B); // Темнее при нажатии
                buttonNext2->Cursor = Cursors::Hand;
                buttonNext2->ForeColor = System::Drawing::Color::Black;
                this->Controls->Add(buttonNext2);
            }
        }

    private:
        nflow_Analyst::flow_Analyst* flow;
        // Обработчики событий
        System::Void OnButtonNextClick(System::Object^ sender, System::EventArgs^ e)
        {
            // Считываем токен из TextBox
            String^ inputText = textBox1->Text;
            std::string token = msclr::interop::marshal_as<std::string>(inputText);
            if (flow->correct_token(token))
            {
                flow->set_token(token);
                currentState = 1;
            }
            else
            {
                currentState = 0;
                MessageBox::Show(L"Неверный токен, попробуйте ещё раз");
            }

            // Переходим к следующему состоянию

            CreateInitialControls();
        }

        System::Void OnButtonSubmitClick(System::Object^ sender, System::EventArgs^ e)
        {
            // Проверяем отмеченные чекбоксы
            String^ selectedItems = L"Выбрано: ";

            if (checkBox1->Checked)
            {
                std::string a = "RU000A106T36";
                flow->add_figi(a);
                selectedItems += checkBox1->Text + L", ";
            }
            if (checkBox2->Checked)
            {
                std::string a = "TCS00A107KX0";
                flow->add_figi(a);
                selectedItems += checkBox2->Text + L", ";
            }
            if (checkBox3->Checked)
            {
                std::string a = "TCS00A105EX7";
                flow->add_figi(a);
                selectedItems += checkBox3->Text + L", ";
            }
            if (checkBox4->Checked)
            {
                std::string a = "TCS00A0JPP37";
                flow->add_figi(a);
                selectedItems += checkBox4->Text + L", ";
            }

            if (selectedItems->Length > 8)
                selectedItems = selectedItems->Remove(selectedItems->Length - 2, 2);

            MessageBox::Show(selectedItems, L"Выбранные акции");

            flow->start();

            currentState = 2;
            CreateInitialControls();

        }

        System::Void OnButtonNext2Click(System::Object^ sender, System::EventArgs^ e)
        {
            // Останавливаем торговый процесс
            flow->end();
            delete flow;
            flow = new nflow_Analyst::flow_Analyst();
            // Возвращаемся к начальному состоянию
            currentState = 0;
            CreateInitialControls();
        }

    };
}
