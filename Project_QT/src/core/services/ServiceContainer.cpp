#include "ServiceContainer.h"
#include "IBrushStateService.h"
#include "IEditorStateService.h"
#include "IClientDataService.h"
#include "IWindowManagerService.h"
#include "IApplicationSettingsService.h"
#include "ILightCalculatorService.h"
#include <QDebug>

namespace RME {
namespace core {

ServiceContainer* ServiceContainer::s_instance = nullptr;

ServiceContainer::ServiceContainer(QObject* parent)
    : QObject(parent)
    , m_brushStateService(nullptr)
    , m_editorStateService(nullptr)
    , m_clientDataService(nullptr)
    , m_windowManagerService(nullptr)
    , m_applicationSettingsService(nullptr)
    , m_lightCalculatorService(nullptr)
{
}

ServiceContainer::~ServiceContainer()
{
    clearAllServices();
}

void ServiceContainer::registerBrushStateService(IBrushStateService* service)
{
    if (m_brushStateService != service) {
        m_brushStateService = service;
        emit serviceRegistered("BrushStateService");
        checkAllServicesRegistered();
        qDebug() << "ServiceContainer: BrushStateService registered";
    }
}

void ServiceContainer::registerEditorStateService(IEditorStateService* service)
{
    if (m_editorStateService != service) {
        m_editorStateService = service;
        emit serviceRegistered("EditorStateService");
        checkAllServicesRegistered();
        qDebug() << "ServiceContainer: EditorStateService registered";
    }
}

void ServiceContainer::registerClientDataService(IClientDataService* service)
{
    if (m_clientDataService != service) {
        m_clientDataService = service;
        emit serviceRegistered("ClientDataService");
        checkAllServicesRegistered();
        qDebug() << "ServiceContainer: ClientDataService registered";
    }
}

void ServiceContainer::registerWindowManagerService(IWindowManagerService* service)
{
    if (m_windowManagerService != service) {
        m_windowManagerService = service;
        emit serviceRegistered("WindowManagerService");
        checkAllServicesRegistered();
        qDebug() << "ServiceContainer: WindowManagerService registered";
    }
}

void ServiceContainer::registerApplicationSettingsService(IApplicationSettingsService* service)
{
    if (m_applicationSettingsService != service) {
        m_applicationSettingsService = service;
        emit serviceRegistered("ApplicationSettingsService");
        checkAllServicesRegistered();
        qDebug() << "ServiceContainer: ApplicationSettingsService registered";
    }
}

void ServiceContainer::registerLightCalculatorService(ILightCalculatorService* service)
{
    if (m_lightCalculatorService != service) {
        m_lightCalculatorService = service;
        emit serviceRegistered("LightCalculatorService");
        checkAllServicesRegistered();
        qDebug() << "ServiceContainer: LightCalculatorService registered";
    }
}

IBrushStateService* ServiceContainer::getBrushStateService() const
{
    return m_brushStateService;
}

IEditorStateService* ServiceContainer::getEditorStateService() const
{
    return m_editorStateService;
}

IClientDataService* ServiceContainer::getClientDataService() const
{
    return m_clientDataService;
}

IWindowManagerService* ServiceContainer::getWindowManagerService() const
{
    return m_windowManagerService;
}

IApplicationSettingsService* ServiceContainer::getApplicationSettingsService() const
{
    return m_applicationSettingsService;
}

ILightCalculatorService* ServiceContainer::getLightCalculatorService() const
{
    return m_lightCalculatorService;
}

bool ServiceContainer::hasBrushStateService() const
{
    return m_brushStateService != nullptr;
}

bool ServiceContainer::hasEditorStateService() const
{
    return m_editorStateService != nullptr;
}

bool ServiceContainer::hasClientDataService() const
{
    return m_clientDataService != nullptr;
}

bool ServiceContainer::hasWindowManagerService() const
{
    return m_windowManagerService != nullptr;
}

bool ServiceContainer::hasApplicationSettingsService() const
{
    return m_applicationSettingsService != nullptr;
}

bool ServiceContainer::hasLightCalculatorService() const
{
    return m_lightCalculatorService != nullptr;
}

bool ServiceContainer::areAllServicesRegistered() const
{
    return hasBrushStateService() &&
           hasEditorStateService() &&
           hasClientDataService() &&
           hasWindowManagerService() &&
           hasApplicationSettingsService() &&
           hasLightCalculatorService();
}

QStringList ServiceContainer::getMissingServices() const
{
    QStringList missing;
    
    if (!hasBrushStateService()) {
        missing << "BrushStateService";
    }
    if (!hasEditorStateService()) {
        missing << "EditorStateService";
    }
    if (!hasClientDataService()) {
        missing << "ClientDataService";
    }
    if (!hasWindowManagerService()) {
        missing << "WindowManagerService";
    }
    if (!hasApplicationSettingsService()) {
        missing << "ApplicationSettingsService";
    }
    if (!hasLightCalculatorService()) {
        missing << "LightCalculatorService";
    }
    
    return missing;
}

void ServiceContainer::clearAllServices()
{
    if (m_brushStateService) {
        emit serviceUnregistered("BrushStateService");
        m_brushStateService = nullptr;
    }
    
    if (m_editorStateService) {
        emit serviceUnregistered("EditorStateService");
        m_editorStateService = nullptr;
    }
    
    if (m_clientDataService) {
        emit serviceUnregistered("ClientDataService");
        m_clientDataService = nullptr;
    }
    
    if (m_windowManagerService) {
        emit serviceUnregistered("WindowManagerService");
        m_windowManagerService = nullptr;
    }
    
    if (m_applicationSettingsService) {
        emit serviceUnregistered("ApplicationSettingsService");
        m_applicationSettingsService = nullptr;
    }
    
    if (m_lightCalculatorService) {
        emit serviceUnregistered("LightCalculatorService");
        m_lightCalculatorService = nullptr;
    }
    
    qDebug() << "ServiceContainer: All services cleared";
}

ServiceContainer* ServiceContainer::instance()
{
    return s_instance;
}

void ServiceContainer::setInstance(ServiceContainer* container)
{
    s_instance = container;
}

void ServiceContainer::checkAllServicesRegistered()
{
    if (areAllServicesRegistered()) {
        emit allServicesRegistered();
        qDebug() << "ServiceContainer: All services are now registered";
    }
}

} // namespace core
} // namespace RME