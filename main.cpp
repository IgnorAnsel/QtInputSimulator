#include <QApplication>
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include <QLabel>
#include <QSlider>
#include <QKeyEvent>
#include <windows.h>
#include <QIcon>
class InputSimulator : public QWidget {
    Q_OBJECT

public:
    InputSimulator(QWidget *parent = nullptr)
        : QWidget(parent), index(0), countdownTime(3), isStopped(false), inputSpeed(100) {
        QVBoxLayout *layout = new QVBoxLayout(this);

        // 输入框
        inputEdit = new QTextEdit(this);
        inputEdit->setPlaceholderText("在此输入你要粘贴的内容");
        layout->addWidget(inputEdit);

        // 倒计时标签
        countdownLabel = new QLabel("点击按钮开始，3秒后开始输入...", this);
        layout->addWidget(countdownLabel);

        // 输入速度标签和滑块
        QLabel *speedLabel = new QLabel("调整输入速度:", this);
        layout->addWidget(speedLabel);

        speedSlider = new QSlider(Qt::Horizontal, this);
        speedSlider->setRange(30, 500); // 输入间隔范围从 30ms 到 500ms
        speedSlider->setValue(inputSpeed); // 初始值
        layout->addWidget(speedSlider);

        // 显示当前速度值
        speedValueLabel = new QLabel("当前速度: " + QString::number(inputSpeed) + " ms", this);
        layout->addWidget(speedValueLabel);

        connect(speedSlider, &QSlider::valueChanged, this, &InputSimulator::updateSpeed);

        // 开始按钮
        QPushButton *startButton = new QPushButton("开始模拟输入", this);
        layout->addWidget(startButton);

        // 停止按钮
        stopButton = new QPushButton("停止输入", this);
        layout->addWidget(stopButton);
        stopButton->setEnabled(false); // 初始状态下禁用停止按钮

        connect(startButton, &QPushButton::clicked, this, &InputSimulator::startCountdown);
        connect(stopButton, &QPushButton::clicked, this, &InputSimulator::stopSimulation);
    }

private slots:
    void startCountdown() {
        text = inputEdit->toPlainText();
        index = 0;
        countdownTime = 3;
        isStopped = false;
        countdownLabel->setText("3秒后开始输入...");
        stopButton->setEnabled(true); // 启用停止按钮

        // 启动倒计时计时器
        countdownTimer = new QTimer(this);
        connect(countdownTimer, &QTimer::timeout, this, &InputSimulator::updateCountdown);
        countdownTimer->start(1000); // 每秒更新一次
    }

    void updateCountdown() {
        if (isStopped) return; // 如果已停止，不再执行

        countdownTime--;
        countdownLabel->setText(QString::number(countdownTime) + "秒后开始输入...");

        if (countdownTime <= 0) {
            countdownTimer->stop(); // 停止倒计时
            simulateKeyPress(); // 开始输入
        }
    }

    void simulateKeyPress() {
        if (isStopped || index >= text.length()) {
            countdownLabel->setText("输入完成！");
            stopButton->setEnabled(false); // 禁用停止按钮
            return; // 输入完毕或已停止
        }

        // 获取当前字符
        QChar ch = text.at(index);

        if (ch == '\n') {
            simulateEnterKey(); // 处理换行符
        } else {
            simulateKey(ch.unicode()); // 正常字符输入
        }

        index++;

        // 使用定时器稍后输入下一个字符
        QTimer::singleShot(inputSpeed, this, &InputSimulator::simulateKeyPress); // 使用输入速度作为间隔
    }

    void simulateKey(ushort unicodeChar) {
        INPUT input;
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = unicodeChar;
        input.ki.time = 0;
        input.ki.dwExtraInfo = 0;

        input.ki.wVk = 0; // 使用 Unicode 字符直接发送
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        SendInput(1, &input, sizeof(INPUT));

        input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    void simulateEnterKey() {
        // 模拟按下回车键
        INPUT input;
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_RETURN; // 使用回车键的虚拟键码
        input.ki.dwFlags = 0;
        SendInput(1, &input, sizeof(INPUT));

        // 模拟释放回车键
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    void updateSpeed(int value) {
        inputSpeed = value;
        speedValueLabel->setText("当前速度: " + QString::number(inputSpeed) + " ms");
    }

    void stopSimulation() {
        isStopped = true;
        if (countdownTimer) {
            countdownTimer->stop(); // 停止倒计时
        }
        countdownLabel->setText("输入已停止");
        stopButton->setEnabled(false); // 禁用停止按钮
    }

private:
    QTextEdit *inputEdit;
    QLabel *countdownLabel;       // 显示倒计时的标签
    QSlider *speedSlider;         // 控制输入速度的滑块
    QLabel *speedValueLabel;      // 显示当前输入速度
    QTimer *countdownTimer;       // 倒计时计时器
    QPushButton *stopButton;      // 停止按钮
    QString text;
    int index;
    int countdownTime;            // 倒计时的时间（秒）
    int inputSpeed;               // 输入速度（毫秒）
    bool isStopped;               // 标志位，表示是否停止输入
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icon.png"));

    InputSimulator simulator;
    simulator.setWindowTitle("InputSimulator");
    simulator.show();

    return app.exec();
}

#include "main.moc"
