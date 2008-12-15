#ifndef _TJUPDATERENGINE_H
#define _TJUPDATERENGINE_H

namespace tj {
	namespace updater {
		class Update;

		class UpdateActions {
			public:
				UpdateActions();
				~UpdateActions();
				bool Run(const Update& parent);
				void Load(TiXmlElement* actions);

			protected:
				struct UpdateAction {
					enum ActionType {
						ActionNothing = 0,
						ActionRun = 1,
					};

					UpdateAction(TiXmlElement* action);
					UpdateAction(ActionType at, const std::wstring& file = L"");
					bool Run(const Update& parent);

					ActionType _type;
					std::wstring _file;
					std::wstring _parameters;
				};

				std::deque<UpdateAction> _actions;
		};

		class Update {
			public:
				Update();
				~Update();
				void Load(TiXmlElement* el);
				const std::wstring& GetDescription() const;
				void DownloadResources();
				bool IsReadyToInstall() const;
				bool GetDownloadedResource(const std::wstring& rid, std::wstring& path) const;
				bool Install();

			private:
				std::wstring _description;
				std::map<std::wstring, std::wstring> _resourcesNeeded;
				std::map<std::wstring, std::wstring> _resourcesDownloaded;
				UpdateActions _actions;
		};

		class UpdatableComponent {
			friend class Updater;
			friend class UpdaterDialog;

			public:
				UpdatableComponent(const std::wstring& name = L"", const std::wstring& versionSource = L"", const std::wstring& licenseSource = L"");
				~UpdatableComponent();
				const std::wstring& GetName() const;
				const std::wstring& GetVersion() const;
				void Load(TiXmlElement* el);
				bool CanBeUpdated() const;
				bool UpdatesAvailable() const;
				void FindAvailableUpdates();
				void DownloadResourcesForUpdates();
				bool CheckedForUpdates() const;
				void SetCheckedForUpdates(bool c);

			protected:
				void CalculateCurrentVersion();

				std::deque<Update> _updates;

			private:
				std::wstring _name;
				std::wstring _versionSource;
				std::wstring _versionHash;
				std::wstring _licenseHash;
				std::wstring _licenseSource;
				std::wstring _updateURL;
				bool _checked;
		};

		class Updater {
			friend class UpdaterDialog;

			public:
				Updater();
				~Updater();
				void Load(TiXmlElement* root);
				void FindAvailableUpdates();
				void Dump();
				bool UpdatesAvailable() const;
				void DownloadResources();
				bool InstallUpdates();

			private:
				std::deque<UpdatableComponent> _components;
		};
	}
}

#endif