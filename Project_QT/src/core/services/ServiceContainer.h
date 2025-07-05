#ifndef RME_SERVICECONTAINER_H
#define RME_SERVICECONTAINER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <memory>

namespace RME {
namespace core {

class IBrushStateService;
class IEditorStateService;
class IClientDataService;
class IWindowManagerService;
class IApplicationSettingsService;
class ILightCalculatorService;

/**
 * @brief Central service registry and dependency injection container
 * 
 * This class manages the lifecycle of all services and provides
 * a centralized way to access them throughout the application.
 * It implements the Service Locator pattern with dependency injection.
 */
class ServiceContainer : public QObject
{
    Q_OBJECT

public:
    explicit ServiceContainer(QObject* parent = nullptr);
    ~ServiceContainer();

    // Service registration
    void registerBrushStateService(IBrushStateService* service);
    void registerEditorStateService(IEditorStateService* service);
    void registerClientDataService(IClientDataService* service);
    void registerWindowManagerService(IWindowManagerService* service);
    void registerApplicationSettingsService(IApplicationSettingsService* service);
    void registerLightCalculatorService(ILightCalculatorService* service);

    // Service access
    IBrushStateService* getBrushStateService() const;
    IEditorStateService* getEditorStateService() const;
    IClientDataService* getClientDataService() const;
    IWindowManagerService* getWindowManagerService() const;
    IApplicationSettingsService* getApplicationSettingsService() const;
    ILightCalculatorService* getLightCalculatorService() const;

    // Service availability checks
    bool hasBrushStateService() const;
    bool hasEditorStateService() const;
    bool hasClientDataService() const;
    bool hasWindowManagerService() const;
    bool hasApplicationSettingsService() const;
    bool hasLightCalculatorService() const;

    // Utility methods
    bool areAllServicesRegistered() const;
    QStringList getMissingServices() const;
    void clearAllServices();

    // Static instance access (optional singleton pattern)
    static ServiceContainer* instance();
    static void setInstance(ServiceContainer* container);

signals:
    void serviceRegistered(const QString& serviceName);
    void serviceUnregistered(const QString& serviceName);
    void allServicesRegistered();

private:
    void checkAllServicesRegistered();

private:
    IBrushStateService* m_brushStateService;
    IEditorStateService* m_editorStateService;
    IClientDataService* m_clientDataService;
    IWindowManagerService* m_windowManagerService;
    IApplicationSettingsService* m_applicationSettingsService;
    ILightCalculatorService* m_lightCalculatorService;

    static ServiceContainer* s_instance;
};

} // namespace core
} // namespace RME

#endif // RME_SERVICECONTAINER_H